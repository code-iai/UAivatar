// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "IAIAvatarCharacter.h"
#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "AIController.h"
#include "UIAIAvatarModule.h" // include for log levels
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/AIModule/Classes/Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/Core/Public/Misc/OutputDeviceNull.h"
#include "Runtime/AIModule/Classes/DetourCrowdAIController.h"
#include "Kismet/GameplayStatics.h"
#include "TaskAnimParamLogic.h"
#include "Engine.h"

//////////////////////////////////////////////////////////////////////////
// AIAIAvatarCharacter

AIAIAvatarCharacter::AIAIAvatarCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	HandTargetPosition.X = 20;
	HandTargetPosition.Y = 56;
	HandTargetPosition.Z = 118;

	RightHandTargetPosition.X = -20;
	RightHandTargetPosition.Y = 56;
	RightHandTargetPosition.Z = 118;

	Spine1Rotation.Roll = 0;
	Spine1Rotation.Pitch = 0;
	Spine1Rotation.Yaw = 0;
	
	Spine2Rotation.Roll = 0;
	Spine2Rotation.Pitch = 0;
	Spine2Rotation.Yaw = 0;

	HipRotation.Roll = 0;
	HipRotation.Pitch = 0;
	HipRotation.Yaw = 0;

	HandRotation.Roll  = -90;
	HandRotation.Pitch = 0;
	HandRotation.Yaw   = 90;

	RightHandRotation.Roll = 90;
	RightHandRotation.Pitch = 0;
	RightHandRotation.Yaw = -90;

	// Current object to pick
	current_obj_target.Key = -1;
	current_obj_target.Value = "";

	isGrasped_l = false;
	isGrasped_r = false;
	isGrasping_l = false;
	isGrasping_r = false;
	graspedObject_r = NULL;
	graspedObject_l = NULL;

}

//////////////////////////////////////////////////////////////////////////
// Input

void AIAIAvatarCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("List", IE_Pressed, this, &AIAIAvatarCharacter::SetTargetObject);

	PlayerInputComponent->BindAction("Pick", IE_Pressed, this, &AIAIAvatarCharacter::GraspTargetObject);
	PlayerInputComponent->BindAction("Pick", IE_Released, this, &AIAIAvatarCharacter::StopHandReach);
	
	PlayerInputComponent->BindAction("GraspObject", IE_Pressed, this, &AIAIAvatarCharacter::StartGraspObject);
	PlayerInputComponent->BindAction("GraspObject", IE_Released, this, &AIAIAvatarCharacter::StopGraspObject);

	PlayerInputComponent->BindAction("GraspObject_r", IE_Pressed, this, &AIAIAvatarCharacter::StartGraspObject_r);
	PlayerInputComponent->BindAction("GraspObject_r", IE_Released, this, &AIAIAvatarCharacter::StopGraspObject_r);

	PlayerInputComponent->BindAction("RaiseHand", IE_Pressed, this, &AIAIAvatarCharacter::StartRaiseHand);
	PlayerInputComponent->BindAction("RaiseHand", IE_Released, this, &AIAIAvatarCharacter::StopRaiseHand);

	PlayerInputComponent->BindAction("RaiseHand_r", IE_Pressed, this, &AIAIAvatarCharacter::StartRaiseHand_r);
	PlayerInputComponent->BindAction("RaiseHand_r", IE_Released, this, &AIAIAvatarCharacter::StopRaiseHand_r);

	PlayerInputComponent->BindAction("DetachObject", IE_Released, this, &AIAIAvatarCharacter::DetachGraspObject);
	PlayerInputComponent->BindAction("DetachObject_r", IE_Released, this, &AIAIAvatarCharacter::DetachGraspObject_r);

	PlayerInputComponent->BindAction("DetachLeftHand", IE_Released, this, &AIAIAvatarCharacter::DetachGraspedObject_l);
	PlayerInputComponent->BindAction("DetachRightHand", IE_Released, this, &AIAIAvatarCharacter::DetachGraspedObject_r);

	PlayerInputComponent->BindAction("AutoMove", IE_Pressed, this, &AIAIAvatarCharacter::SimpleMoveToTargetPoint);

	PlayerInputComponent->BindAxis("MoveForward", this, &AIAIAvatarCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AIAIAvatarCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	//PlayerInputComponent->BindAxis("Turn", this, &AIAIAvatarCharacter::TurnCam);
	PlayerInputComponent->BindAxis("TurnRate", this, &AIAIAvatarCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUp", this, &AIAIAvatarCharacter::LookCam);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AIAIAvatarCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AIAIAvatarCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AIAIAvatarCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AIAIAvatarCharacter::OnResetVR);
}

void AIAIAvatarCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AIAIAvatarCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AIAIAvatarCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AIAIAvatarCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AIAIAvatarCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

/*void AIAIAvatarCharacter::TurnCam(float Axis)
{

	FRotator HeadRotation;
	FQuat CameraRotation;

	if (HeadFollowCamera) {

		CameraRotation = FollowCamera->GetComponentRotation().Quaternion();
		HeadRotation = this->GetMesh()->GetComponentTransform().InverseTransformRotation(CameraRotation).Rotator();
		HeadRotation += FRotator(0, -90, 0);

		if (HeadRotation.Yaw > 90 || HeadRotation.Yaw < -180)
			HeadRotation = FRotator(0, 0, 0);
		if (HeadRotation.Yaw < -90)
			HeadRotation = FRotator(0, 0, 0);

		Cast<UIAIAvatarAnimationInstance>(this->GetMesh()->GetAnimInstance())->SkelControl_Head = HeadRotation;
	}

	if (Axis != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddYawInput(Axis);

	}
}*/

/*void AIAIAvatarCharacter::LookCam(float Axis)
{

	FRotator HeadRotation;
	FQuat CameraRotation;
	if (HeadFollowCamera) {

		CameraRotation = FollowCamera->GetComponentRotation().Quaternion();
		HeadRotation = this->GetMesh()->GetComponentTransform().InverseTransformRotation(CameraRotation).Rotator();
		HeadRotation += FRotator(0, -90, 0);

		if (HeadRotation.Yaw > 90 || HeadRotation.Yaw < -180)
			HeadRotation = FRotator(0, 0, 0);
		if (HeadRotation.Yaw < -90)
			HeadRotation = FRotator(0, 0, 0);

		Cast<UIAIAvatarAnimationInstance>(this->GetMesh()->GetAnimInstance())->SkelControl_Head = HeadRotation;
	}

	if (Axis != 0.f && Controller && Controller->IsLocalPlayerController())
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddPitchInput(Axis);
	}
}*/

 bool AIAIAvatarCharacter::ControlledByAI() {
	 check(GetController());
	 return GetController()->IsA(AAIController::StaticClass());
 }

 bool AIAIAvatarCharacter::ControlledByPlayer() {
	 check(GetController());
	 return GetController()->IsA(APlayerController::StaticClass());
 }


 // Reach for an Actor. Modifies the l_hand bone
 void AIAIAvatarCharacter::LeftHandReachForActor(AActor* Actor) {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Setting IK to GraspObject Location"));

	 FTransform graspObjectTransform = Actor->GetActorTransform();
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Setting GraspObject Location to %s"), *graspObjectTransform.ToString());

	 // Calculate transform between bone and world space
	 // Alternatively you can also set the IK coord space to "World space"
	 check(GetMesh());
	 FVector GraspObjectLocationInComponentSpace = GetMesh()->GetComponentTransform().InverseTransformPosition(graspObjectTransform.GetLocation());

	 StartLeftHandReach(GraspObjectLocationInComponentSpace);
 }

 // Reach to specific coordinates. Modifies the l_hand bone
 void AIAIAvatarCharacter::StartLeftHandReach(FVector Target) {

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 AnimationInstance->LeftHandIKTargetPosition = Target;
	 IKEnableActive = true;
	 IKEnableTickDirection = 1;
 }

 // Reach to specific coordinates. Modifies the r_hand bone
 void AIAIAvatarCharacter::StartRightHandReach(FVector Target) {

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 AnimationInstance->RightHandIKTargetPosition = Target;
	 IKEnableActive_r = true;
	 IKEnableTickDirection_r = 1;
 }

 // Stop hands reaching motion
 void AIAIAvatarCharacter::StopHandReach() {

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 ObjectToGrasp = NULL;

	 IKEnableActive = true;
	 IKEnableTickDirection = -1;
	 IKEnableActive_r = true;
	 IKEnableTickDirection_r = -1;
 }

 // Stop left hand reaching motion
 void AIAIAvatarCharacter::StopLeftHandReach() {

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 IKEnableActive = true;
	 IKEnableTickDirection = -1;
 }

 // Stop right hand reaching motion
 void AIAIAvatarCharacter::StopRightHandReach() {

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 IKEnableActive_r = true;
	 IKEnableTickDirection_r = -1;
 }

 void AIAIAvatarCharacter::StartGraspObject() {

	 UE_LOG(LogAvatarCharacter, Log, TEXT("START GraspObject Left Hand"));

	 check(GetMesh());

	 auto MyMesh = GetMesh();

	 if (ObjectToGrasp)
	 {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Setting IK to GraspObject Location"));

		 // Get Animation
		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance != nullptr);

		 FTransform graspObjectTransform = ObjectToGrasp->GetActorTransform();
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Setting GraspObject Location to %s"), *graspObjectTransform.ToString());
		 
		 // Calculate transform between bone and world space
		 // Alternatively you can also set the IK coord space to "World space"
		 FVector GraspObjectLocationInComponentSpace = MyMesh->GetComponentTransform().InverseTransformPosition(graspObjectTransform.GetLocation());

		 AnimationInstance->Spine1Rotation = FRotator(0, 20, 0);
		 AnimationInstance->Spine2Rotation = FRotator(0, 0, 0);
		 AnimationInstance->HipRotation    = FRotator(0, 0, 0);

		 StartLeftHandReach(GraspObjectLocationInComponentSpace);

		 //IKEnableActive = true;
		 //IKEnableTickDirection = 1;

	 }

 }

 void AIAIAvatarCharacter::StartGraspObject_r() {

	 UE_LOG(LogAvatarCharacter, Log, TEXT("START GraspObject Right Hand"));

	 check(GetMesh());

	 auto MyMesh = GetMesh();

	 if (ObjectToGrasp_r)
	 {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Setting IK to GraspObject Location"));

		 // Get Animation
		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance != nullptr);

		 FTransform graspObjectTransform = ObjectToGrasp_r->GetActorTransform();
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Setting GraspObject Location to %s"), *graspObjectTransform.ToString());

		 // Calculate transform between bone and world space
		 // Alternatively you can also set the IK coord space to "World space"
		 FVector GraspObjectLocationInComponentSpace = MyMesh->GetComponentTransform().InverseTransformPosition(graspObjectTransform.GetLocation());

		 AnimationInstance->Spine1Rotation = FRotator(20, 20, -40);
		 AnimationInstance->Spine2Rotation  = FRotator(0, 0, 0);
		 AnimationInstance->HipRotation     = FRotator(0, 0, 0);

		 StartRightHandReach(GraspObjectLocationInComponentSpace);
	 }

 }

 // List reachable objects
 TMap<FString, FHitResult> AIAIAvatarCharacter::ListObjects() {
	 
	 // Local variables
	 TArray<FHitResult> HitResults;
	 FTransform ObjectTransform;
	 FVector ObjLocationInCompSpace;
	 FString obj_name;
	 TMap<FString, FHitResult> MyUniqueHits;

	 HitResults = TraceObjectsWithBP();

	 check(GetMesh());

	 int index_count = 0;
	 for (auto It = HitResults.CreateIterator(); It; It++)
	 {

		 // Getting location relative to component
		 ObjLocationInCompSpace = GetMesh()->GetComponentTransform().InverseTransformPosition((*It).ImpactPoint);
		 obj_name = (*It).Actor->GetName();

		 // Get Animation
		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance != nullptr);
		 
		 int check_from = 0;

		 if (AnimationInstance->bActivateSitAnim) {
			 check_from = -10;
		 }

		 // Verifying locations
		 if (ObjLocationInCompSpace.Y > check_from && ObjLocationInCompSpace.Z > -6.5) {			// Front and over surface

			 if (!MyUniqueHits.Contains(obj_name)) { // Ensure unique objects
				 MyUniqueHits.Emplace(obj_name, *It); // Add to list
			 }
		 }
	 }

	 return MyUniqueHits;
 }

 // List reachable objects and set a target
 void AIAIAvatarCharacter::SetTargetObject() {

	 // Local variables
	 TMap<FString, FHitResult> MyUniqueHits;
	 TMap<int, FString> ObjectsIndex;

	 MyUniqueHits = ListObjects();

	 // Printing objects into screen
	 int index_count = 0;
	 for (auto It = MyUniqueHits.CreateIterator(); It; ++It) {
		 ObjectsIndex.Emplace(index_count, (*It).Key);
		 
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, FString::Printf(TEXT("  - %s"), *(*It).Key), true, FVector2D(1.2, 1.2));
		 UE_LOG(LogAvatarCharacter, Log, TEXT("%d %s"), index_count, *(*It).Key);

		 index_count++;
	 }
	 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, "Object List:", true, FVector2D(1.5, 1.5));

	 current_obj_target.Key++;
	 if (MyUniqueHits.Num() != 0) {
		 if (current_obj_target.Key >= MyUniqueHits.Num()) {
			 current_obj_target.Key = 0;
		 }
		 current_obj_target.Value = ObjectsIndex[current_obj_target.Key];
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10, FColor::Green, FString::Printf(TEXT("Target Object: %s"), *current_obj_target.Value), true, FVector2D(1.2, 1.2));
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Target Object #%d: %s"), current_obj_target.Key + 1, *current_obj_target.Value);
	 }
	 else {
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, "No graspable objects", true, FVector2D(1.7, 1.7));
		 UE_LOG(LogAvatarCharacter, Log, TEXT("No graspable objects"));
		 current_obj_target.Key = -1;
	 }
 }

 // List reachable objects and start grasp if target object is reachable 
 void AIAIAvatarCharacter::GraspTargetObject() {

	 // Local variables
	 TMap<FString, FHitResult> MyUniqueHits;

	 MyUniqueHits = ListObjects();

	 // Verify if target is valid
	 if (current_obj_target.Key != -1) {

		 // Verify list hasn't changed
		 if (MyUniqueHits.FindRef(current_obj_target.Value).GetActor() == NULL) {		 
			 UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Sorry. The object list changed."));
			 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, "ERROR: Sorry. The object list changed.", true, FVector2D(1.7, 1.7));
		 }
		 else {
			 UE_LOG(LogAvatarCharacter, Log, TEXT("Grasping target: %s "), *current_obj_target.Value);
			 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, FString::Printf(TEXT("Grasping target: %s"), *current_obj_target.Value), true, FVector2D(1.2, 1.2));
			 //StartGrasp(MyUniqueHits[current_obj_target.Value]);
			 UTaskAnimParamLogic* AnimLogic = Cast<UTaskAnimParamLogic>(GetComponentByClass(UTaskAnimParamLogic::StaticClass()));
			 check(AnimLogic);
			 AnimLogic->CallGraspingAnimChain(current_obj_target.Value,"any");
		 }
	 }
	 else {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: There is no target object."));
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, "ERROR: There is no target object.", true, FVector2D(1.7, 1.7));
	 }
 }

 // List reachable objects and start grasp if target object is reachable 
 TArray<FString>  AIAIAvatarCharacter::GraspTargetObject_ROS(FString targetObject, bool hold) {

	 // Local variables
	 TMap<FString, FHitResult> MyUniqueHits;
	 TArray<FString> objectList;

	 MyUniqueHits = ListObjects();
 
	 // Verify list hasn't changed
	 if (MyUniqueHits.FindRef(targetObject).GetActor() == NULL) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Object \"%s\" not found!"),*targetObject);
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: Object \"%s\" not found!"), *targetObject), true, FVector2D(1.7, 1.7));
	 }
	 else {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Grasping: %s "), *targetObject);
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, FString::Printf(TEXT("Grasping: %s"), *targetObject), true, FVector2D(1.2, 1.2));
		 StartGrasp(MyUniqueHits[targetObject], hold);
	 }

	 for (auto It = MyUniqueHits.CreateIterator(); It; ++It) {
		 objectList.Add((*It).Key);
	 }

	 return objectList;
 }

// Start general grasp
#pragma optimize("", off)
void AIAIAvatarCharacter::StartGrasp(FHitResult object, bool hold) {

	// Local variables
	float distance;
	float horizontal_distance;
	FTransform ObjectTransform;
	FRotator GraspingRotation;
	FVector  Direction;
	FVector ObjLocationInCompSpace;
	FVector AproachLocationInCompSpace;
	FVector RetireLocationInCompSpace;
	FVector HoldingLocationInCompSpace;
	FVector ShouldersLocation = FVector(0, 0, 154);

	FTimerHandle *HandIKTimeHandle_target = new FTimerHandle();
	FTimerHandle HandIKTimeHandle_mid;
	FTimerHandle HandIKTimeHandle_end;
	FTimerHandle HandRotTimeHandle_end;
	FTimerHandle SpineTimeHandle_end;
	FTimerHandle AttachTimeHandle;

	FTimerDelegate HandIKSetDelegate_target;
	FTimerDelegate HandIKSetDelegate_mid;
	FTimerDelegate HandIKSetDelegate_end;
	FTimerDelegate HandIKRotDelegate_end;
	FTimerDelegate HandRotDelegate_end;
	FTimerDelegate SpineDelegate_end;
	FTimerDelegate AttachDelegate;

	check(GetMesh());

	// Get Animation
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance != nullptr);

	// Getting location relative to component
	ObjLocationInCompSpace = GetMesh()->GetComponentTransform().InverseTransformPosition(object.GetActor()->GetActorLocation());

	// Getting total distance
	FVector tmp1;
	Direction = ObjLocationInCompSpace - ShouldersLocation;
	Direction.ToDirectionAndLength(tmp1, distance);

	// Getting horizontal distance
	ObjLocationInCompSpace.ToDirectionAndLength(tmp1, horizontal_distance);

	// Calculate "usual" rotation to pickup an object. 
	float angle = Direction.Rotation().Yaw - 90;
	if (angle > 40){
		AnimationInstance->Spine1Rotation.Roll = angle - 40;
	}
	else if (angle < -20 ){
		AnimationInstance->Spine1Rotation.Roll = angle + 20;
	}
	else {
		AnimationInstance->Spine1Rotation.Roll = 0;
	}

	// Calculate Bowing
	if ((distance > 55) && (distance <= 120)) {
		AnimationInstance->HipRotation.Roll = 0;
		AnimationInstance->Spine1Rotation.Yaw = (distance - 55) / 70 * 65;
	}
	else if ((distance > 120) && (distance <= 150)) {
		AnimationInstance->HipRotation.Roll = 90;
		AnimationInstance->Spine1Rotation.Yaw = (distance - 120) / 50 * 45;
	}
	else if (distance > 150) {
		AnimationInstance->HipRotation.Roll = 90;
		AnimationInstance->Spine1Rotation.Yaw = 45;
	}
	else {
		AnimationInstance->HipRotation.Roll = 0;
		AnimationInstance->Spine1Rotation.Yaw = 0;
	}

	// Reset previous rotation for grasping
	AnimationInstance->Spine2Rotation.Roll = 0;

	// Verify my hands are not busy
	if (isGrasped_l && isGrasped_r) {
		UE_LOG(LogAvatarCharacter, Log, TEXT("Both my hands are busy! I'm not an octopus!!"));
	}
	else {
		bool use_left = false;
		if (isGrasped_r) {		// Use left hand if right hand is busy,
			use_left = true;	// no matter where the object is.
		} else if (ObjLocationInCompSpace.X > 0 && !isGrasped_l) {			
			use_left = true;   // If right hand is free, then only use left hand 
		}						// as loong as it is also free and object is on left side.

		ObjectToGrasp = object.GetActor();

		// Correction for the wrist
		ObjLocationInCompSpace.Z += 4;
		ObjLocationInCompSpace.Y -= 7;

		
		
		RetireLocationInCompSpace.Y = 14;

		if (object.GetActor()->ActorHasTag("Bowl")) {
			if (use_left) {
				GraspingRotation = FRotator(-10, 70, 180);
				ObjLocationInCompSpace += FVector(7, -5, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(5, -15, 15);
			}
			else {
				GraspingRotation = FRotator(10, -70, 0);
				ObjLocationInCompSpace += FVector(-7, -5, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(-5, -15, 15);
			}
			RetireLocationInCompSpace = ObjLocationInCompSpace + FVector(0, -15, 5);
		}
		else if (object.GetActor()->ActorHasTag("SpoonSoup")) {
			if (use_left) {
				GraspingRotation = FRotator(9, 100, 25);
				ObjLocationInCompSpace += FVector(4, -8, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(5, -15, 15);

			}
			else {
				GraspingRotation = FRotator(9, -100, -25);
				ObjLocationInCompSpace += FVector(-4, -8, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(-5, -15, 15);
			}
			RetireLocationInCompSpace = ObjLocationInCompSpace + FVector(0, -15, 5);
		}
		else if (object.GetActor()->ActorHasTag("DinnerFork")) {
			if (use_left) {
				GraspingRotation = FRotator(20, 90, 0);
				ObjLocationInCompSpace += FVector(4, -8, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(5, -15, 15);

			}
			else {
				GraspingRotation = FRotator(20, -90, 0);
				ObjLocationInCompSpace += FVector(-4, -8, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(-5, -15, 15);
			}
			RetireLocationInCompSpace = ObjLocationInCompSpace + FVector(0, -15, 5);
		}
		else if (object.GetActor()->ActorHasTag("Jug")) {
			if (use_left) {
				GraspingRotation = FRotator(5, 100, -10);
				ObjLocationInCompSpace += FVector(7, -10, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(5, -15, 15);

			}
			else {
				GraspingRotation = FRotator(5, -100, 10);
				ObjLocationInCompSpace += FVector(-7, -10, -1);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(-5, -15, 15);
			}
			RetireLocationInCompSpace = ObjLocationInCompSpace + FVector(0, -15, 5);
		}
		else if (object.GetActor()->ActorHasTag("BreadKnife") || object.GetActor()->ActorHasTag("CookKnife")) {
			if (use_left) {
				GraspingRotation = FRotator(50, 170, -10);
				ObjLocationInCompSpace += FVector(4, -8, 5);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(3, -20, 20);
				RetireLocationInCompSpace = ObjLocationInCompSpace + FVector(3, -20, 15);

			}
			else {
				GraspingRotation = FRotator(50, -170, -10);
				ObjLocationInCompSpace += FVector(-4, -8, 5);
				AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(-3, -20, 20);
				RetireLocationInCompSpace = ObjLocationInCompSpace + FVector(-3, -20, 15);
			}
		}
		else {
			if (use_left) {
				GraspingRotation = FRotator(0, 70, -90);
			}
			else {
				GraspingRotation = FRotator(0, -70, 90);
			}
			AproachLocationInCompSpace = ObjLocationInCompSpace + FVector(0, -15, 15);
			RetireLocationInCompSpace = ObjLocationInCompSpace + FVector(0, -15, 5);
		}
		UE_LOG(LogAvatarCharacter, Log, TEXT("Grasping Rotation %f %f %f"),GraspingRotation.Pitch,GraspingRotation.Yaw,GraspingRotation.Roll);

		if (use_left) {

			HoldingLocationInCompSpace = FVector(15, 10, 105);

			// 0s) Rotate hand to grasping pose now
			StartLeftHandRotationEnablement(GraspingRotation);
			// 0s) Move hand to aproach location now
			StartLeftHandIKEnablement(AproachLocationInCompSpace);

			// Defining delegates
			HandIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftHandIKTo, ObjLocationInCompSpace);
			HandIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftHandIKTo, RetireLocationInCompSpace);

			if (hold) {
				HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftHandIKTo, HoldingLocationInCompSpace);
			}
			else {
				HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartLeftHandIKDisablement);
				HandRotDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartLeftHandRotationDisablement);
			}

			isGrasping_l = true;
		}
		else {

			HoldingLocationInCompSpace = FVector(-15, 15, 105);

			// 0s) Rotate hand to grasping pose now
			StartRightHandRotationEnablement(GraspingRotation);

			// 0s) Move hand to aproach location now
			StartRightHandIKEnablement(AproachLocationInCompSpace);

			// Defining Delegates
			HandIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, ObjLocationInCompSpace);
			HandIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, RetireLocationInCompSpace);
			if (hold) {
				HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, HoldingLocationInCompSpace);
			}
			else {
				HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartRightHandIKDisablement);
				HandRotDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartRightHandRotationDisablement);
			}

			isGrasping_r = true;
		}

		// 0s) Rotate spine to reach object
		StartSpineEnablement(AnimationInstance->Spine1Rotation, AnimationInstance->Spine2Rotation, AnimationInstance->HipRotation);


		// 0.5s) Move hand to object location
		GetWorldTimerManager().SetTimer(*HandIKTimeHandle_target, HandIKSetDelegate_target, 5, false, 0.5);

		// 1.5s) Attach
		AttachDelegate = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::AttachObjectCPP);
		GetWorldTimerManager().SetTimer(AttachTimeHandle, AttachDelegate, 5, false, 1.5);

		// 1.5s) Move back to mid point 
		GetWorldTimerManager().SetTimer(HandIKTimeHandle_mid, HandIKSetDelegate_mid, 5, false, 1.5);

		// 2.2s) Rotate hand back
		if(!hold) {
			GetWorldTimerManager().SetTimer(HandRotTimeHandle_end, HandRotDelegate_end, 5, false, 2.2);
		}

		// 2.2s) Move hand back or move hand to holding position
		GetWorldTimerManager().SetTimer(HandIKTimeHandle_end, HandIKSetDelegate_end, 5, false, 2.2);

		// 2.2s) Rotate spine back
		SpineDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartSpineDisablement);
		GetWorldTimerManager().SetTimer(SpineTimeHandle_end, SpineDelegate_end, 5, false, 2.2);
	}
}
#pragma optimize("", on)

// Place object on left hand over/in specified place 
void AIAIAvatarCharacter::PlaceObject(FString targetPlace, FString Hand, FVector targetPoint) {
	
	// Local variables
	TMap<FString, FHitResult> MyUniqueHits;
	FRotator placingRotation;
	FRotator TargetRotation;
	FVector TargetLocation;
	FVector AproachLocation;
	FVector RetireLocation;
	//FVector Origin;   In case we are checking the table and selectnig a place over it 
	//FVector Bounds;
	//FBox PlaceBox;
	//FHitResult Place;

	FTimerHandle HandIKTimeHandle_target;
	FTimerHandle HandIKTimeHandle_mid;
	FTimerHandle HandIKTimeHandle_end;
	FTimerHandle HandRotTimeHandle_end;
	FTimerHandle SpineTimeHandle_end;
	FTimerHandle DetachTimeHandle;

	FTimerDelegate HandIKSetDelegate_target;
	FTimerDelegate HandIKSetDelegate_mid;
	FTimerDelegate HandIKSetDelegate_end;
	FTimerDelegate HandRotDelegate_end;
	FTimerDelegate SpineDelegate_end;
	FTimerDelegate DetachDelegate;


	// Check hands and if they have any object
	bool goWithRightHand = false;

	if (Hand.Equals("right")) {
		if (!isGrasped_r) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: There is nothing to place from right hand."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("ERROR: There is nothing to place from right hand."), true, FVector2D(1.5, 1.5));

			return;
		}
		else {
			goWithRightHand = true;
		}
	}
	else if (Hand.Equals("left")) {
		if (!isGrasped_l) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: There is nothing to place from left hand."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("ERROR: There is nothing to place from left hand."), true, FVector2D(1.5, 1.5));

			return;
		}
	}
	else if (Hand.Equals("any")) {
		if (isGrasped_r) {
			goWithRightHand = true;
		}
		else if (!isGrasped_l) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: There is nothing to place from any hand."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("ERROR: There is nothing to place from any hand."), true, FVector2D(1.5, 1.5));

			return;
		}
	}

	// Get Animation
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance != nullptr);

	// Defining Spine Rotation
	AnimationInstance->HipRotation.Roll = 0;
	AnimationInstance->Spine1Rotation.Yaw = 40;

	// Reset previous rotation for grasping
	AnimationInstance->Spine1Rotation.Roll = 0;
	AnimationInstance->Spine2Rotation.Roll = 0;

	// Define middle point locations and default location.
	if (goWithRightHand) {
		TargetLocation = FVector(-30, 42, 90);
		RetireLocation = FVector(-22, 17, 100);
		AproachLocation = FVector(-22, 17, 105);
	}
	else {
		TargetLocation = FVector(30, 42, 90);
		RetireLocation = FVector(22, 17, 100);
		AproachLocation = FVector(22, 17, 105);
	}
	
	if (targetPlace.Equals("VECTOR")) {

		FVector socketAbsLoc;
		FVector loc;
		FVector offset;

		UE_LOG(LogAvatarCharacter, Error, TEXT("Target Abs Location \"%f %f %f\"."), targetPoint.X, targetPoint.Y, targetPoint.Z);
		if (goWithRightHand) {
			socketAbsLoc = GetMesh()->GetBoneLocation(TEXT("hand_r"));
			UE_LOG(LogAvatarCharacter, Error, TEXT("Hand Abs Location \"%f %f %f\"."), socketAbsLoc.X, socketAbsLoc.Y, socketAbsLoc.Z);

			loc = graspedObject_r->GetActorLocation();
			UE_LOG(LogAvatarCharacter, Error, TEXT("Bowl Abs location   \"%f %f %f\"."), loc.X, loc.Y, loc.Z);

			offset = graspedObject_r->GetActorLocation() - socketAbsLoc;
			UE_LOG(LogAvatarCharacter, Error, TEXT("Offset              \"%f %f %f\"."), offset.X, offset.Y, offset.Z);

		}
		else {
			socketAbsLoc = GetMesh()->GetBoneLocation(TEXT("hand_l"));
			UE_LOG(LogAvatarCharacter, Error, TEXT("Hand Abs Location \"%f %f %f\"."), socketAbsLoc.X, socketAbsLoc.Y, socketAbsLoc.Z);

			loc = graspedObject_l->GetActorLocation();
			UE_LOG(LogAvatarCharacter, Error, TEXT("Bowl Abs location   \"%f %f %f\"."), loc.X, loc.Y, loc.Z);

			offset = graspedObject_l->GetActorLocation() - socketAbsLoc;
			UE_LOG(LogAvatarCharacter, Error, TEXT("Offset              \"%f %f %f\"."), offset.X, offset.Y, offset.Z);

		}
		targetPoint -= offset;
		UE_LOG(LogAvatarCharacter, Error, TEXT("Target Abs Location \"%f %f %f\"."), targetPoint.X, targetPoint.Y, targetPoint.Z);

		TargetLocation = GetMesh()->GetComponentTransform().InverseTransformPosition(targetPoint);
		UE_LOG(LogAvatarCharacter, Error, TEXT("Target Location     \"%f %f %f\"."), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);

	}
	else if (targetPlace.Equals("microwave")) {	
		MyUniqueHits = ListObjects();

		// Verify microwave is near
		if (MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor() == NULL) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: Microwave is not near you."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: The Microwave is not near you.")), true, FVector2D(1.7, 1.7));

			return;
		}
		else {
			// Getting location relative to component
			TargetLocation = MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor()->GetActorLocation();
			TargetRotation = MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor()->GetActorRotation();
			TargetLocation += TargetRotation.RotateVector(FVector(0, -5, -10));
			AnimationInstance->Spine1Rotation.Yaw = 47;
			TargetLocation = GetMesh()->GetComponentTransform().InverseTransformPosition(TargetLocation);
		}
	}
	else if (targetPlace.Equals("table")) {
		// No need for anything else
	}
	else {
		UE_LOG(LogAvatarCharacter, Warning, TEXT("Warning: Unrecognized place \"%s\". Just placing here."), *targetPlace);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, FString::Printf(TEXT("Warning: Unrecognized place \"%s\". Just placing here."), *targetPlace), true, FVector2D(1.5, 1.5));
		AnimationInstance->HipRotation.Roll = 90;
		AnimationInstance->Spine1Rotation.Yaw = 45;
		TargetLocation.Z -= 90;
	}

	if (goWithRightHand) {

		// Define Rotation
		if (graspedObject_r->ActorHasTag("Bowl")) {
			placingRotation = FRotator(10, -70, 0);
			RetireLocation.X -= 20;
		}
		else {
			placingRotation = FRotator(0, -70, 90);
		}

		// 0s) Rotate hand to grasping pose now
		if (RightHandRotationAlphaInterpolation.TargetValue != 1) {
			StartRightHandRotationEnablement(placingRotation);
		}

		// 0s) Move hand to aproach location now
		if (RightHandIKAlphaInterpolation.TargetValue != 1) {
			StartRightHandIKEnablement(AproachLocation);
		}
		else {
			InterpolateRightHandIKTo(AproachLocation);
		}

		// Setting delegates
		HandIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, TargetLocation);
		DetachDelegate = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::DetachGraspedObject_r);
		HandIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, RetireLocation);
		HandRotDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartRightHandRotationDisablement);
		HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartRightHandIKDisablement);
		SpineDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartSpineDisablement);

		isGrasping_r = false;
	}
	else {

		// Define Rotation
		if (graspedObject_l->ActorHasTag("Bowl")) {
			placingRotation = FRotator(-10, 70, 180);
			RetireLocation.X += 10;
			TargetLocation.Z -= 0;
		}
		else {
			placingRotation = FRotator(0, 70, -90);
		}

		// 0s) Rotate hand to grasping pose now
		if (LeftHandRotationAlphaInterpolation.TargetValue != 1) {
			StartLeftHandRotationEnablement(placingRotation);
		}

		// 0s) Move hand to aproach location now
		if (LeftHandIKAlphaInterpolation.TargetValue != 1) {
			StartLeftHandIKEnablement(AproachLocation);
		}
		else {
			InterpolateLeftHandIKTo(AproachLocation);
		}

		// Setting delegates
		HandIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftHandIKTo, TargetLocation);
		DetachDelegate = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::DetachGraspedObject_l);
		HandIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftHandIKTo, RetireLocation);
		HandRotDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartLeftHandRotationDisablement);
		HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartLeftHandIKDisablement);
		SpineDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartSpineDisablement);

		isGrasping_l = false;
	}

	// 0s) Rotate spine to reach object
	StartSpineEnablement(AnimationInstance->Spine1Rotation, AnimationInstance->Spine2Rotation, AnimationInstance->HipRotation);

	// 0.5s) Move hand to target location
	GetWorldTimerManager().SetTimer(HandIKTimeHandle_target, HandIKSetDelegate_target, 5, false, 0.5);

	// 1s) Detach 
	GetWorldTimerManager().SetTimer(DetachTimeHandle, DetachDelegate, 5, false, 1.8);

	// 2s) Move back to mid point 
	GetWorldTimerManager().SetTimer(HandIKTimeHandle_mid, HandIKSetDelegate_mid, 5, false, 2);

	// 2.7s) Rotate hand back
	GetWorldTimerManager().SetTimer(HandRotTimeHandle_end, HandRotDelegate_end, 5, false, 2.7);

	// 2.7s) Move hand back or move hand to holding position
	GetWorldTimerManager().SetTimer(HandIKTimeHandle_end, HandIKSetDelegate_end, 5, false, 2.7);

	// 2.7s) Rotate spine back
	GetWorldTimerManager().SetTimer(SpineTimeHandle_end, SpineDelegate_end, 5, false, 2.7);

}

// Press a button on the microwave
void AIAIAvatarCharacter::PressMicrowaveButton(FString button) {

	// Local variables
	TMap<FString, FHitResult> MyUniqueHits;
	FRotator ObjRotation;
	FVector ObjLocation;
	FVector MidLocation;
	FVector ButtonLocation;

	FTimerHandle FingerIKTimeHandle_target;
	FTimerHandle FingerIKTimeHandle_mid;
	FTimerHandle FingerIKTimeHandle_end;
	FTimerHandle SpineTimeHandle_end;

	FTimerDelegate FingerIKSetDelegate_target;
	FTimerDelegate FingerIKSetDelegate_mid;
	FTimerDelegate FingerIKSetDelegate_end;
	FTimerDelegate SpineDelegate_end;
	
	AActor *Microwave;
	
	MyUniqueHits = ListObjects();

	Microwave = MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor();

	// Verify microwave is near
	if (Microwave == nullptr) {
		UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: The microwave is not near to you."));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: The microwave is not near to you.")), true, FVector2D(1.7, 1.7));
	}
	else {

		// Get Animation
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance != nullptr);

		// Getting location
	
		ObjLocation = Microwave->GetActorLocation();
		ObjRotation = Microwave->GetActorRotation();

		if (button.Equals("open")) {
			ButtonLocation = Microwave->GetTransform().TransformPosition(FVector(21, 17.83, -12.84));
			MidLocation = Microwave->GetTransform().TransformPosition(FVector(21, 30, -12.84));
		}
		else if (button.Equals("start"))
		{
			ButtonLocation = Microwave->GetTransform().TransformPosition(FVector(23.07, 17.83, -8.5));
			MidLocation = Microwave->GetTransform().TransformPosition(FVector(23.07, 30, -8.5));

		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: Unrecognized button \"%s\"."), *button);
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("Error: Unrecognized button \"%s\"."), *button), true, FVector2D(1.7, 1.7));
			return;
		}
		
		//ButtonLocation =  this->GetActorTransform().InverseTransformPosition(ObjLocation);

		// Defining Spine Rotation
		AnimationInstance->HipRotation.Roll = 0;
		AnimationInstance->Spine1Rotation.Yaw = 38; // changed for demo/yasmin

		// Reset previous rotation for grasping
		AnimationInstance->Spine1Rotation.Roll = 0;
		AnimationInstance->Spine2Rotation.Roll = 0;

		// Define mid location
		//MidLocation = ButtonLocation;
		//MidLocation.Y -= 15;

		// Using right or left hand
		if (!isGrasped_r) {
			// 0s) Move Finger to mid location
			StartRightFingerIKEnablement(MidLocation);

			FingerIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightFingerIKTo, ButtonLocation);
			FingerIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightFingerIKTo, MidLocation);
			FingerIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartRightFingerIKDisablement);
		}
		else if (!isGrasped_l) {
			// 0s) Move Finger to mid location
			StartLeftFingerIKEnablement(MidLocation);

			FingerIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftFingerIKTo, ButtonLocation);
			FingerIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftFingerIKTo, MidLocation);
			FingerIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartLeftFingerIKDisablement);
		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: Both hands are busy."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("Error: Both hands are busy."), true, FVector2D(1.7, 1.7));
			return;
		}

		// 0s) Rotate spine to reach object
		StartSpineEnablement(AnimationInstance->Spine1Rotation, AnimationInstance->Spine2Rotation, AnimationInstance->HipRotation);

		// 1s) Move Finger to target location
		GetWorldTimerManager().SetTimer(FingerIKTimeHandle_target, FingerIKSetDelegate_target, 5, false, 1);

		// 2.7s) Move back to mid point 
		GetWorldTimerManager().SetTimer(FingerIKTimeHandle_mid, FingerIKSetDelegate_mid, 5, false, 2.7);

		// 2.9s) Move finger back
		GetWorldTimerManager().SetTimer(FingerIKTimeHandle_end, FingerIKSetDelegate_end, 5, false, 2.9);

		// 2.7s) Rotate spine back
		SpineDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartSpineDisablement);
		GetWorldTimerManager().SetTimer(SpineTimeHandle_end, SpineDelegate_end, 5, false, 2.7);
	}
}

// Close a door
void AIAIAvatarCharacter::CloseDoor(FString door) {

	// Local variables
	TMap<FString, FHitResult> MyUniqueHits;
	FRotator Hand_Rotation;
	FRotator ObjRotation;
	FVector ObjLocation;
	FVector MidLocation;
	FVector DoorLocation;

	FTimerHandle HandIKTimeHandle_target;
	FTimerHandle HandIKTimeHandle_mid;
	FTimerHandle HandIKTimeHandle_end;
	FTimerHandle SpineTimeHandle_end;
	FTimerHandle HandRotTimeHandle_end;

	FTimerDelegate HandIKSetDelegate_target;
	FTimerDelegate HandIKSetDelegate_mid;
	FTimerDelegate HandIKSetDelegate_end;
	FTimerDelegate SpineDelegate_end;
	FTimerDelegate HandRotDelegate_end;

	MyUniqueHits = ListObjects();

	// Get Animation
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance != nullptr);

	if (door.Equals("microwave")) {
		// Verify microwave is near
		if (MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor() == NULL) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: The microwave is not near to you."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: The microwave is not near to you.")), true, FVector2D(1.7, 1.7));
			return;
		}
		else {

			// Getting location
			ObjLocation = MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor()->GetActorLocation();
			ObjRotation = MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor()->GetActorRotation();
			
			ObjLocation += ObjRotation.RotateVector(FVector(-12, 57, 0));
			MidLocation = ObjLocation;
			MidLocation += ObjRotation.RotateVector(FVector(-20, 10, 0));
			DoorLocation = GetMesh()->GetComponentTransform().InverseTransformPosition(ObjLocation);

			// Define mid location
			MidLocation = GetMesh()->GetComponentTransform().InverseTransformPosition(MidLocation);

		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: Unrecognized door \"%s\"."), *door);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("Error: Unrecognized door \"%s\"."), *door), true, FVector2D(1.7, 1.7));
		return;
	}

	// Using right or left hand
	if (!isGrasped_l) {
		
		Hand_Rotation = FRotator(0, 45,180);

		// 0s) Move Hand to mid location
		StartLeftHandIKEnablement(MidLocation);
		StartLeftHandRotationEnablement(Hand_Rotation);

		HandRotDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartLeftHandRotationDisablement);
		HandIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftHandIKTo, DoorLocation);
		HandIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateLeftHandIKTo, MidLocation);
		HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartLeftHandIKDisablement);
	}
	else if (!isGrasped_r) {

		Hand_Rotation = FRotator(0, -45, 0);

		// 0s) Move Hand to mid location
		StartRightHandIKEnablement(MidLocation);
		StartRightHandRotationEnablement(Hand_Rotation);

		HandRotDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartRightHandRotationDisablement);
		HandIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, DoorLocation);
		HandIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, MidLocation);
		HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartRightHandIKDisablement);
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: Both hands are busy."));
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("Error: Both hands are busy."), true, FVector2D(1.7, 1.7));
		return;
	}

	// 1s) Move hand to target location
	GetWorldTimerManager().SetTimer(HandIKTimeHandle_target, HandIKSetDelegate_target, 5, false, 1);

	// 2.7s) Move hand back
	GetWorldTimerManager().SetTimer(HandIKTimeHandle_end, HandIKSetDelegate_end, 5, false, 2.7);

	// 2.7s) Move rotation back
	GetWorldTimerManager().SetTimer(HandRotTimeHandle_end, HandRotDelegate_end, 5, false, 2.5);

}

void AIAIAvatarCharacter::HighLightObject(FString Object_Name, bool OnOff) {

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), OutActors);
	for (int32 Index = 0; Index < OutActors.Num(); ++Index) {
		AActor* Comp = OutActors[Index];
		if (Comp->GetName().Equals(Object_Name)) {
			UStaticMeshComponent *temp = Cast<UStaticMeshComponent>(Comp->GetComponentByClass(UStaticMeshComponent::StaticClass()));
			temp->SetRenderCustomDepth(OnOff);
		}
	}
}

// Process Console Command to call respective avatar function
void AIAIAvatarCharacter::ProcessConsoleCommand(FString inLine) {
	 
	if (!(inLine.IsEmpty())) {

		TArray<FString> tokens;
		inLine.ParseIntoArrayWS(tokens, NULL, true);

		UTaskAnimParamLogic* AnimLogic = Cast<UTaskAnimParamLogic>(GetComponentByClass(UTaskAnimParamLogic::StaticClass()));
		check(AnimLogic);

		UE_LOG(LogAvatarCharacter, Log, TEXT("Processing Command %s"), *inLine);

		if (tokens.Num() == 1) {
			// Listing objects
			if (tokens[0].Equals("list")) {
				SetTargetObject();
			}
			// Sit
			else if (tokens[0].Equals("sit")) {
				// Get Animation
				UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
				check(AnimationInstance != nullptr);
				AnimationInstance->bActivateSitAnim = true;
			}
			// Stopping hand raise
			else if (tokens[0].Equals("drop")) {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, \
					"Warning: Stopping raise on left and right hands.",			\
					true, FVector2D(1.7, 1.7));
				StopRaiseHand_r();
				StopRaiseHand();
			}
			// Stop reaching or holding
			else if (tokens[0].Equals("release")) {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, \
					"Warning: Stopping reach from left and right hands",		\
					true, FVector2D(1.5, 1.5));
				StopHandReach();
			}
			// Detaching object
			else if (tokens[0].Equals("detach")) {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, \
					"Warning: Detaching from right and left hands.",			\
					true, FVector2D(1.5, 1.5));
				DetachGraspedObject_r();
				DetachGraspedObject_l();
			}
			// Command not found
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
					"ERROR: Command \"%s\" not found."), *tokens[0]), true, FVector2D(1.7, 1.7));
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,						\
					"One word commands are: spoon, list, drop, release and detach",		\
					true, FVector2D(1.5, 1.5));
			}
		}
		else if (tokens.Num() == 2) {
			// Grasp specified object
			if (tokens[0].Equals("grasp")) {
				AnimLogic->CallGraspingAnimChain(tokens[1], "any");
				//GraspTargetObject_ROS();
			}
			// Stand Up
			else if (tokens[0].Equals("stand") && tokens[1].Equals("up")) {
				// Get Animation
				UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
				check(AnimationInstance != nullptr);
				AnimationInstance->bActivateSitAnim = false;
			}
			// Feed Person
			else if (tokens[0].Equals("feed")) {
				AnimLogic->CallFeedingAnimChain(tokens[1]);
			}
			// Raise Hand
			else if (tokens[0].Equals("raise")) {
				// Right
				if (tokens[1].Equals("right")) {
					StartRaiseHand_r();
				}
				// Left
				else if (tokens[1].Equals("left")) {
					StartRaiseHand();
				}
				// None
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Can't process argument \"%s\".\n Try: raise <left|right>"),				\
						*tokens[1]), true, FVector2D(1.7, 1.7));
				}				
			}
			// Stopping hand raise
			else if (tokens[0].Equals("drop")) {
				// Right
				if (tokens[1].Equals("right")) {
					StopRaiseHand_r();
				}
				// Left
				else if (tokens[1].Equals("left")) {
					StopRaiseHand();
				}
				// None
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Can't process argument \"%s\".\n Try: drop <left|right>"),				\
						*tokens[1]), true, FVector2D(1.7, 1.7));
				}
			}
			// Stop reaching or holding
			else if (tokens[0].Equals("release")) {
				// Right
				if (tokens[1].Equals("right")) {
					StopRightHandReach();
				}
				// Left
				else if (tokens[1].Equals("left")) {
					StopLeftHandReach();
				}
				// None
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Can't process argument \"%s\".\n Try: release <left|right>"),			\
						*tokens[1]), true, FVector2D(1.7, 1.7));
				}
			}
			// Detaching object
			else if (tokens[0].Equals("detach")) {
				// Right
				if (tokens[1].Equals("right")) {
					DetachGraspedObject_r();
				}
				// Left
				else if (tokens[1].Equals("left")) {
					DetachGraspedObject_l();
				}
				// None
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Can't process argument \"%s\".\n Try: detach <left|right>"),			\
						*tokens[1]), true, FVector2D(1.7, 1.7));
				}
			}
			// Press microwave button
			else if (tokens[0].Equals("press")) {
				PressMicrowaveButton(tokens[1]);
			}
			else if (tokens[0].Equals("close")) {
				CloseDoor(tokens[1]);
			}
			// Placing objects
			else if (tokens[0].Equals("place")) {
				AnimLogic->StartPlacingAnimChain(tokens[1], "any");
			// Path following
			} else if (tokens[0].Equals("follow")) {
				StartPathFollowing(tokens[1]);
			}
			// Forking
			else if (tokens[0].Equals("fork")) {
				AnimLogic->CallForkAnimation(tokens[1]);
			}
			// Spooning
			else if (tokens[0].Equals("spoon")) {
				AnimLogic->CallSpoonAnimation(tokens[1]);
			}
			// Wrong amount of tagerts
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(					\
					"ERROR: No matching command for: %s %s"), *tokens[0], *tokens[1]),								\
					true, FVector2D(1.7, 1.7));
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,										\
					"Two words commands are: grasp, raise, drop, release, press, close, place, follow and detach",	\
					true, FVector2D(1.5, 1.5));
			}
		}
		else if (tokens.Num() == 3) {
			// Avatar rotation
			if (tokens[0].Equals("turn") && tokens[1].Equals("to")) {
				if (tokens[2].IsNumeric()) {
					TurnTo("angle",FCString::Atof(*tokens[2]));
				}
				else if (tokens[2].Equals("camera")) {
					TurnTo("camera");
				}
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Can't process argument \"%s\".\n Try an <angle value> or \"camera\""),	\
						*tokens[1]), true, FVector2D(1.7, 1.7));
				}
			}
			// Slide object
			else if (tokens[0].Equals("slide")) {
				AnimLogic->CallSlideObject(tokens[1],tokens[2]);
			}
			// Move to left sit
			else if (tokens[0].Equals("move") && tokens[1].Equals("sit") && tokens[2].Equals("left")) {
				UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
				check(AnimationInstance != nullptr);
				AnimationInstance->bActivateMoveToLeftSit = true;
			}
			// Cut
			else if (tokens[0].Equals("cut")) {
				if (tokens[2].IsNumeric()) {
					AnimLogic->CallSlicingAnimChain(tokens[1], FCString::Atof(*tokens[2]));
				}
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(\
						"ERROR: Width argument %s is not a numeric value."), \
						*tokens[2]), true, FVector2D(1.7, 1.7));
				}
			}
			// Highlight object
			else if (tokens[0].Equals("highlight")) {
				if (tokens[2].Equals("on")) {
					HighLightObject(tokens[1], true);
				}
				else {
					HighLightObject(tokens[1], false);
				}
			}
			// Pass Page
			else if (tokens[0].Equals("pass") && tokens[1].Equals("page")) {
				AnimLogic->CallPassPageAnimChain(tokens[2]);
			}
			// Close Book
			else if (tokens[0].Equals("close") && tokens[1].Equals("book")) {
				AnimLogic->CallCloseBookAnimChain(tokens[2]);
			}
			// Point Book
			else if (tokens[0].Equals("read") && (tokens[1].Equals("left") || tokens[1].Equals("right"))) {
				AnimLogic->CallReadBookAnimChain(tokens[2], tokens[1]);
			}
			// Interpolation off spine
			else if	(tokens[0].Equals("interpolation") && tokens[1].Equals("off") && tokens[2].Equals("spine")) {
				StartSpineDisablement();
			}
			// Grasp specified object with specified hand
			else if (tokens[0].Equals("grasp") && (tokens[1].Equals("right") || tokens[1].Equals("left") || tokens[1].Equals("any"))) {
				AnimLogic->CallGraspingAnimChain(tokens[2], tokens[1]);
			}
			// Placing objects
			else if (tokens[0].Equals("place")) {
				// Right or left 
				if (tokens[1].Equals("right") || tokens[1].Equals("left")) {
					AnimLogic->StartPlacingAnimChain(tokens[2], tokens[1]);
				}
				// None
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(			\
						"ERROR: Can't process argument \"%s\".\n Try: place <left|right> <where to place>"),	\
						*tokens[1]), true, FVector2D(1.7, 1.7));
				}
			}
			// Reset look to
			else if (tokens[0].Equals("look") && tokens[1].Equals("to")) {
				if (tokens[2].Equals("front")) {
					AnimLogic->StartReleaseLookAnimation();
				}
				else {

					TArray<AActor*> FoundActors;
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
					bool bFound = false;

					for (auto It = FoundActors.CreateIterator(); It; ++It) {
						if ((*It)->GetName().Equals(tokens[2])) {
							bFound = true;
							AnimLogic->StartLookAnimation((*It)->GetActorLocation());
							break;
						}
					}

					// Verify list hasn't changed
					if (!bFound){
						UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Object \"%s\" not found!"), *tokens[2]);
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: Object \"%s\" not found!"), *tokens[2]), true, FVector2D(1.7, 1.7));
						
					}
				}
			}
			// Pouring
			else if (tokens[0].Equals("pour") && tokens[1].Equals("over")) {
				AnimLogic->CallPourAnimation(tokens[2]);
			}
			// None
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
					"ERROR: No matching command for: %s %s %s"),									\
					*tokens[0], *tokens[1], *tokens[2]), true, FVector2D(1.7, 1.7));
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,						\
					"Three words command are: interpolation, turn to, place, look to front and grasp.",			\
					true, FVector2D(1.5, 1.5));
			}
		}
		else if (tokens.Num() == 4) {
			// Interpolate off hands
			if (tokens[0].Equals("interpolation") && tokens[1].Equals("off")) {
				// IK
				if (tokens[3].Equals("handIK")) {
					// Right
					if (tokens[2].Equals("right")) {
						StartRightHandIKDisablement();
					}
					// Left
					else if (tokens[2].Equals("left")) {
						StartLeftHandIKDisablement();
					}
					// None
					else {
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
							"ERROR: Can't process argument \"%s\".\n										\
							Try: interpolation off <left|right> handIK"),									\
							*tokens[3]), true, FVector2D(1.7, 1.7));
					}
				}
				// Rotation
				else if (tokens[3].Equals("handRot")) {
					// Right
					if (tokens[2].Equals("right")) {
						StartRightHandRotationDisablement();
					}
					// Left
					else if (tokens[2].Equals("left")) {
						StartLeftHandRotationDisablement();
					}
					// None
					else {
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
							"ERROR: Can't process argument \"%s\".\n										\
							Try: interpolation off <left|right> handRot"),									\
								*tokens[3]), true, FVector2D(1.7, 1.7));
					}
				} 
				// None
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Can't process argument \"%s\".\n										\
						Try: interpolation off <left|right> <handIK|handRot> "),						\
						*tokens[2]), true, FVector2D(1.7, 1.7));
				}
			}
			// Grasp specified object with specified hand
			else if (tokens[0].Equals("grasp") && (tokens[1].Equals("right") || tokens[1].Equals("left") || tokens[1].Equals("any"))) {
				AnimLogic->CallGraspingAnimChain(tokens[2], tokens[1]);
			}
			// Hand Reaching
			else if (tokens[1].Equals("hand") && tokens[2].Equals("reach")) {
				if (tokens[0].Equals("right")) {
					AnimLogic->StartHandReachAnimation(true, tokens[3]);
				}
				else {
					AnimLogic->StartHandReachAnimation(false, tokens[3]);
				}
			}
			// Placing objects
			else if (tokens[0].Equals("place")) {
				if (tokens[1].IsNumeric() && tokens[2].IsNumeric() && tokens[3].IsNumeric()) {

					FVector target;
					target.X = FCString::Atof(*tokens[1]);
					target.Y = FCString::Atof(*tokens[2]);
					target.Z = FCString::Atof(*tokens[3]);

					// Right or Left
					AnimLogic->StartPlacingAnimChain("VECTOR", "any", target);

				}
				// No numeric vector
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(\
						"ERROR: Make sure your rotator (%s %s %s) only have numeric values"), \
						*tokens[1], *tokens[2], *tokens[3]), true, FVector2D(1.7, 1.7));
				}
			}
			// None
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
					"ERROR: No matching command for: %s %s %s %s"),									\
					*tokens[0], *tokens[1], *tokens[2], *tokens[3]), true, FVector2D(1.7, 1.7));
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,						\
					"4 words command are: interpolation off <left|right> <handIK|handRot>         \n\
					                      place <x> <y> <z> \n", \
					true, FVector2D(1.5, 1.5));
			}
		}
		else if (tokens.Num() == 5) {
			// Placing objects
			if (tokens[0].Equals("place")) {
				if (tokens[2].IsNumeric() && tokens[3].IsNumeric() && tokens[4].IsNumeric()) {

					FVector target;
					target.X = FCString::Atof(*tokens[2]);
					target.Y = FCString::Atof(*tokens[3]);
					target.Z = FCString::Atof(*tokens[4]);

					// Right or Left
					if (tokens[1].Equals("right") || tokens[1].Equals("left")) {
						AnimLogic->StartPlacingAnimChain("VECTOR", tokens[1], target);
					}
					// None
					else {
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(\
							"ERROR: Can't process argument \"%s\".\n Try: place <left|right> <x> <y> <z>"), \
							*tokens[1]), true, FVector2D(1.7, 1.7));
					}
				}
				// No numeric vector
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(\
						"ERROR: Make sure your rotator (%s %s %s) only have numeric values"), \
						*tokens[2], *tokens[3], *tokens[4]), true, FVector2D(1.7, 1.7));
				}
			}
			// looking at
			else if (tokens[0].Equals("look") && tokens[1].Equals("to")) {
				if (tokens[2].IsNumeric() && tokens[3].IsNumeric() && tokens[4].IsNumeric()) {
					
					FVector target;
					target.X = FCString::Atof(*tokens[2]);
					target.Y = FCString::Atof(*tokens[3]);
					target.Z = FCString::Atof(*tokens[4]);

					AnimLogic->StartLookAnimation(target);
				}
				// No numeric vector
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(\
						"ERROR: Make sure your vector (%s %s %s) only have numeric values"), \
						*tokens[2], *tokens[3], *tokens[4]), true, FVector2D(1.7, 1.7));
				}
			}
			// None 
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(\
					"ERROR: No matching command for: %s %s %s %s %s"), \
					*tokens[0], *tokens[1], *tokens[2], *tokens[3], *tokens[4]), true, FVector2D(1.7, 1.7));
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, \
					"5 words command are: look to <x> <y> <z>         \n\
					                      place <left|right> <x> <y> <z> ", \
					true, FVector2D(1.5, 1.5));
			}
		}
		else if (tokens.Num() == 6) {
			// Interpolate on|edit spine
			if (tokens[0].Equals("interpolation") && tokens[5].Equals("spine")) {
				if (tokens[2].IsNumeric() && tokens[3].IsNumeric() && tokens[4].IsNumeric()) {

					FRotator target;
					target.Pitch = FCString::Atof(*tokens[2]);
					target.Yaw = FCString::Atof(*tokens[3]);
					target.Roll = FCString::Atof(*tokens[4]);

					// Spine
					if (tokens[1].Equals("start")) {
						StartSpineEnablement(target);
					}
					else if (tokens[1].Equals("set")) {
						InterpolateSpineTo(target);
					}
					// None
					else {
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
							"ERROR: Can't process argument \"%s\".\n										\
							Try: interpolation <start|set> <x> <y> <z> spine"),								\
							*tokens[1]), true, FVector2D(1.7, 1.7));
					}
				}
				// No numeric vector
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Make sure your rotator (%s %s %s) only have numeric values"),			\
						*tokens[2], *tokens[3], *tokens[4]), true, FVector2D(1.7, 1.7));
				}
			}
			// None
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
					"ERROR: No matching command for: %s %s.\n										\
					Try: interpolation <start|set> <x> <y> <z> spine"),								\
					*tokens[0], *tokens[1]), true, FVector2D(1.7, 1.7));
			}
		} else if (tokens.Num() == 7) {
			// Interpolation on|set left|right handRot|handIK
			// Interpolation set spine offset
			if (tokens[0].Equals("interpolation")) {
				if (tokens[2].IsNumeric() && tokens[3].IsNumeric() && tokens[4].IsNumeric()) {

					FVector target;
					target.X = FCString::Atof(*tokens[2]);
					target.Y = FCString::Atof(*tokens[3]);
					target.Z = FCString::Atof(*tokens[4]);

					// Right
					if (tokens[5].Equals("right")) {
						// IK
						if (tokens[6].Equals("handIK")) {
							if (tokens[1].Equals("start")) {
								StartRightHandIKEnablement(target);
							}
							else if (tokens[1].Equals("set")) {
								InterpolateRightHandIKTo(target);
							}
							else {
								GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
									"ERROR: Can't process argument \"%s\".\n										\
									Try: interpolation <start|set>  <x> <y> <z> right handIK"), *tokens[1]),			\
									true, FVector2D(1.7, 1.7));
							}
						}
						// Rotation
						else if (tokens[6].Equals("handRot")) {
							if (tokens[1].Equals("start")) {
								StartRightHandRotationEnablement(FRotator(target.X, target.Y, target.Z));
							}
							else if (tokens[1].Equals("set")) {
								InterpolateRightHandRotationTo(FRotator(target.X, target.Y, target.Z));
							}
							else {
								GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
									"ERROR: Can't process argument \"%s\".\n										\
									Try: interpolation <start|set> <x> <y> <z> right handRot"),						\
									*tokens[1]), true, FVector2D(1.7, 1.7));
							}
						}
						// None
						else {
							GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
								"ERROR: Can't process argument \"%s\"."),										\
								*tokens[6]), true, FVector2D(1.7, 1.7));
							GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,						\
								"Arguments could be: handIK, handRot.", true, FVector2D(1.5, 1.5));
						}
					}
					// Left
					else if (tokens[5].Equals("left")) {
						// IK
						if (tokens[6].Equals("handIK")) {
							if (tokens[1].Equals("start")) {
								StartLeftHandIKEnablement(target);
							}
							else if (tokens[1].Equals("set")) {
								InterpolateLeftHandIKTo(target);
							}
							else {
								GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
									"ERROR: Can't process argument \"%s\".\n										\
									Try: interpolation <start|set> <x> <y> <z> left handIK "							\
									), *tokens[1]), true, FVector2D(1.7, 1.7));
							}
						}
						// Rotation
						else if (tokens[6].Equals("handRot")) {
							if (tokens[1].Equals("start")) {
								StartLeftHandRotationEnablement(FRotator(target.X, target.Y, target.Z));
							}
							else if (tokens[1].Equals("set")) {
								InterpolateLeftHandRotationTo(FRotator(target.X, target.Y, target.Z));
							}
							else {
								GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
									"ERROR: Can't process argument \"%s\".\n										\
									Try: interpolation <start|set> <x> <y> <z> left handRot"),							\
									*tokens[1]), true, FVector2D(1.7, 1.7));
							}
						}
						// None
						else {
							GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
								"ERROR: Can't process argument \"%s\"."),										\
								*tokens[6]), true, FVector2D(1.7, 1.7));
							GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,						\
								"Arguments could be: handIK, handRot.", true, FVector2D(1.5, 1.5));
						}
					}
					// Spine
					else if (   tokens[1].Equals("set") && 
								tokens[5].Equals("spine") && 
						        tokens[6].Equals("offset")) {

						FRotator offset = SpineBoneInterpolation.TargetValue;
						offset.Add(target.X, target.Y, target.Z);
						InterpolateSpineTo(offset);
					}
					// None
					else {
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
							"ERROR: Can't process argument \"%s\"."),										\
							*tokens[5]), true, FVector2D(1.7, 1.7));
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,						\
							"7 words commands are:\n														\
							1) interpolation set <x> <y> <z> spine offset\n									\
							2) interpolation <start|set> <x> <y> <z> <left|right> <handIK|handRot>",		\
							true, FVector2D(1.5, 1.5));
					}
				}
				// No numeric vector
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Make sure your rotator (%s %s %s) only have numeric values"),			\
						*tokens[2], *tokens[3], *tokens[4]), true, FVector2D(1.7, 1.7));
				}
			}
			// Wrong command
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
					"ERROR: No matching command for: %s %s"),										\
					*tokens[0], *tokens[1]), true, FVector2D(1.7, 1.7));
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow,						\
					"7 words commands are:\n														\
					1) interpolation set <x> <y> <z> spine offset\n									\
					2) interpolation <on|set> <x> <y> <z> <left|right> <handIK|handRot>",			\
					true, FVector2D(1.5, 1.5));
			}
		}
		else if (tokens.Num() == 8) {
			// Interpolate set 
			if (tokens[0].Equals("interpolation") && tokens[1].Equals("set") && tokens[7].Equals("offset")) {
				if (tokens[2].IsNumeric() && tokens[3].IsNumeric() && tokens[4].IsNumeric()) {

					FVector target;
					target.X = FCString::Atof(*tokens[3]);
					target.Y = FCString::Atof(*tokens[4]);
					target.Z = FCString::Atof(*tokens[5]);

					// Right
					if (tokens[5].Equals("right")) {
						// IK
						if (tokens[6].Equals("handIK")) {
							target += RightHandIKBoneInterpolation.TargetValue;
							InterpolateRightHandIKTo(target);

						}					
						// Rotation
						else if (tokens[6].Equals("handRot")) {
							FRotator offset = RightHandRotationBoneInterpolation.TargetValue;
							offset.Add(target.X, target.Y, target.Z);
							InterpolateRightHandRotationTo(offset);
						}

					}
					// Left
					else if (tokens[5].Equals("left")) {
						// IK
						if (tokens[6].Equals("handIK")) {
							target += LeftHandIKBoneInterpolation.TargetValue;
							InterpolateLeftHandIKTo(target);
						}	
						// Rotation
						else if (tokens[6].Equals("handRot")) {
							FRotator offset = LeftHandRotationBoneInterpolation.TargetValue;
							offset.Add(target.X, target.Y, target.Z);
							InterpolateLeftHandRotationTo(offset);
						}
					}			
					// None
					else {
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
							"ERROR: Can't process argument \"%s\".											\
							Try: interpolation set <x> <y> <z> <left|right> <handRot|handIK> offset"),		\
							*tokens[6]), true, FVector2D(1.7, 1.7));
					}
				}
				// No numeric vector
				else {
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
						"ERROR: Make sure your rotator (%s %s %s) only have numeric values"),			\
						*tokens[3], *tokens[4], *tokens[5]), true, FVector2D(1.7, 1.7));
				}
			}
			// Wrong command
			else {
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
					"ERROR: No matching command for: %s %s.											\
					Try: interpolation set <x> <y> <z> <left|right> <handRot|handIK> offset"),		\
					*tokens[0], *tokens[1]), true, FVector2D(1.7, 1.7));
			}
		}
		// No command found
		else {
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT(	\
				"ERROR: No matching command with so many arguments (+8)."),						\
				*tokens[0]), true, FVector2D(1.7,1.7));
		}
 	}
}

#pragma optimize("", off)
 void AIAIAvatarCharacter::AttachObjectCPP() {

	 if (isGrasping_l || isGrasping_r) {

		 // Attachment variables
		 FVector  NewObjLocation = FVector(0, 0, 0);
		 FRotator NewObjRotation = FRotator(0, 0, 0);
		 UStaticMeshComponent *ObjectMesh;
		 FFingerRots_t NewFingerRots;
		 FName    socket;

		 FAttachmentTransformRules attachRules = FAttachmentTransformRules(
			 EAttachmentRule::SnapToTarget,
			 EAttachmentRule::SnapToTarget,
			 EAttachmentRule::KeepWorld,
			 true);

		 // Default Fingers' Rotations
		 // Thumb
		 NewFingerRots.thumb_01 = FRotator(-63, -48, 71);
		 NewFingerRots.thumb_02 = FRotator(0, -10, 0);
		 NewFingerRots.thumb_03 = FRotator(0, -7, 0);
		 // Index
		 NewFingerRots.index_01 = FRotator(-3, 0, 0);
		 NewFingerRots.index_02 = FRotator(0, -40, 0);
		 NewFingerRots.index_03 = FRotator(0, -60, 0);
		 // Middle
		 NewFingerRots.middle_02 = FRotator(0, -55, 0);
		 NewFingerRots.middle_03 = FRotator(0, -55, 0);
		 // Ring
		 NewFingerRots.ring_01 = FRotator(3, 0, 0);
		 NewFingerRots.ring_02 = FRotator(0, -45, 0);
		 NewFingerRots.ring_03 = FRotator(0, -60, 0);
		 // Pinky
		 NewFingerRots.pinky_01 = FRotator(5, 0, 0);
		 NewFingerRots.pinky_02 = FRotator(0, -30, 10);
		 NewFingerRots.pinky_03 = FRotator(0, -45, 0);

		 // Get Animation
		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance != nullptr);

		 // Get Static Meshes
		 check(GetMesh());
		 auto AvatarMesh = GetMesh();

		 ObjectMesh = Cast<UStaticMeshComponent>(ObjectToGrasp->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		 ObjectMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

		 // Object specific settings
		 if (ObjectToGrasp->ActorHasTag(TEXT("MilkBox"))) {				// ** Milk Box **
			 
			 // Attaching Rule
			 attachRules.RotationRule = EAttachmentRule::KeepRelative;
			 
			 // Object Location
			 NewObjLocation = FVector(-3, 5.5, 0);
			 
			 // Fingers' Rotations
			 // Thumb
			 NewFingerRots.thumb_01 = FRotator(-63, -48, 71);
			 NewFingerRots.thumb_02 = FRotator(0, -10, 0);
			 NewFingerRots.thumb_03 = FRotator(0, -7, 0);
			 // Index
			 NewFingerRots.index_01 = FRotator(-3, 0, 0);
			 NewFingerRots.index_02 = FRotator(0, -40, 0);
			 NewFingerRots.index_03 = FRotator(0, -60, 0);
			 // Middle
			 NewFingerRots.middle_02 = FRotator(0, -55, 0);
			 NewFingerRots.middle_03 = FRotator(0, -55, 0);
			 // Ring
			 NewFingerRots.ring_01 = FRotator(3, 0, 0);
			 NewFingerRots.ring_02 = FRotator(0, -45, 0);
			 NewFingerRots.ring_03 = FRotator(0, -60, 0);
			 // Pinky
			 NewFingerRots.pinky_01 = FRotator(5, 0, 0);
			 NewFingerRots.pinky_02 = FRotator(0, -30, 10);
			 NewFingerRots.pinky_03 = FRotator(0, -45, 0);

			 // Object Rotation
			 if (isGrasping_r) {
				 NewObjRotation = FRotator(-10, 180, 0);
			 } else {
				 NewObjRotation = FRotator(10, 0, -180);
			 }
		 }
		 else if (ObjectToGrasp->ActorHasTag(TEXT("BreadKnife"))) {		// ********** Bread Knife *********

			 // Object Location
			 NewObjLocation = FVector(-2, 2, 9);

			 // Fingers' Rotations
 			 // Thumb
			 NewFingerRots.thumb_01 = FRotator(-45, -48, 90);
			 NewFingerRots.thumb_02 = FRotator(25, -55, 0);
			 NewFingerRots.thumb_03 = FRotator(0, -45, 0);
			 // Index
			 NewFingerRots.index_01 = FRotator(-3, -80, 0);
			 NewFingerRots.index_02 = FRotator(0, -90, 0);
			 NewFingerRots.index_03 = FRotator(0, -45, 0);
			 // Middle
			 NewFingerRots.middle_01 = FRotator(0, -90, 0);
			 NewFingerRots.middle_02 = FRotator(0, -90, 0);
			 NewFingerRots.middle_03 = FRotator(0, -45, 0);
			 // Ring
			 NewFingerRots.ring_01 = FRotator(3, -90, 0);
			 NewFingerRots.ring_02 = FRotator(0, -90, 0);
			 NewFingerRots.ring_03 = FRotator(0, -45, 0);
			 // Pinky
			 NewFingerRots.pinky_01 = FRotator(5, -100, 0);
			 NewFingerRots.pinky_02 = FRotator(0, -80, 10);
			 NewFingerRots.pinky_03 = FRotator(0, -45, 0);
			 
			 // Object Rotation
			 if (isGrasping_r) {
				 NewObjRotation = FRotator(70, -180, -90);
			 } else {
				 NewObjRotation = FRotator(288, 0, -255);
			 }
		 }
		 else if (ObjectToGrasp->ActorHasTag(TEXT("CookKnife"))) {		// ********** Cook Knife *********

			 // Fingers' Rotations
			 // Thumb
			 NewFingerRots.thumb_01 = FRotator(-45, -48, 90);
			 NewFingerRots.thumb_02 = FRotator(25, -55, 0);
			 NewFingerRots.thumb_03 = FRotator(0, -45, 0);
			 // Index
			 NewFingerRots.index_01 = FRotator(-3, -80, 0);
			 NewFingerRots.index_02 = FRotator(0, -90, 0);
			 NewFingerRots.index_03 = FRotator(0, -45, 0);
			 // Middle
			 NewFingerRots.middle_01 = FRotator(0, -90, 0);
			 NewFingerRots.middle_02 = FRotator(0, -90, 0);
			 NewFingerRots.middle_03 = FRotator(0, -45, 0);
			 // Ring
			 NewFingerRots.ring_01 = FRotator(3, -90, 0);
			 NewFingerRots.ring_02 = FRotator(0, -90, 0);
			 NewFingerRots.ring_03 = FRotator(0, -45, 0);
			 // Pinky
			 NewFingerRots.pinky_01 = FRotator(5, -100, 0);
			 NewFingerRots.pinky_02 = FRotator(0, -80, 10);
			 NewFingerRots.pinky_03 = FRotator(0, -45, 0);

			 // Object Location
			 NewObjLocation = FVector(-4, 2, 10);

			 if (isGrasping_r) {
				 // Object Rotation
				 NewObjRotation = FRotator(110.5,0, 90);
			 }
			 else {
				 NewObjRotation = FRotator(-69.5, 0, 90);
			 }
		 }
		 else if (ObjectToGrasp->ActorHasTag(TEXT("SpoonSoup"))) {		// ********* Spoon Soup *********
			
			// Collision
			//#if WITH_FLEX
			//ObjectMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Flex, ECollisionResponse::ECR_Block);
			//#endif
			 // Object Location		 // ud  lr  bf
			 NewObjLocation = FVector(-1.9, 7.6, 0.3);
			 // Finger's Rotations
			 // Thumb
			 NewFingerRots.thumb_01 = FRotator(-34, -20, 71);
			 NewFingerRots.thumb_02 = FRotator(0, -46, 0);
			 NewFingerRots.thumb_03 = FRotator(0, -23, 0);
			 // Index
			 NewFingerRots.index_01 = FRotator(0, -50, -12);
			 NewFingerRots.index_02 = FRotator(0, -62, 0);
			 NewFingerRots.index_03 = FRotator(0, -50, 0);
			 // Middle
			 NewFingerRots.middle_01 = FRotator(0, -60, -10);
			 NewFingerRots.middle_02 = FRotator(0, -70, 0);
			 NewFingerRots.middle_03 = FRotator(0, -60, 0);
			 // Ring
			 NewFingerRots.ring_01 = FRotator(0, -70, -10);
			 NewFingerRots.ring_02 = FRotator(0, -70, 0);
			 NewFingerRots.ring_03 = FRotator(0, -60, 0);
			 // Pinky
			 NewFingerRots.pinky_01 = FRotator(0, -90, -25);
			 NewFingerRots.pinky_02 = FRotator(0, -60, 0);
			 NewFingerRots.pinky_03 = FRotator(0, -60, 0);
	 
			 // Object Rotation
			 if (isGrasping_r) {      // p   y  r
				 NewObjRotation = FRotator(-36, 101, -24);
			 } else {
				 NewObjRotation = FRotator(28, -120, 180);
			 }
		 }
		 else if (ObjectToGrasp->ActorHasTag(TEXT("DinnerFork"))) {		// ********* Dinner Fork *********
		 
			 // Object Rotation
			 if (isGrasping_r) {      // p   y  r

				 // Object Location		 // ud  lr  bf
				 NewObjLocation = FVector(-1.9,	7.6, 0.3);
				 // Finger's Rotations
				 // Thumb
				 NewFingerRots.thumb_01 = FRotator(-34, -20, 71);
				 NewFingerRots.thumb_02 = FRotator(0, -46, 0);
				 NewFingerRots.thumb_03 = FRotator(0, -23, 0);
				 // Index
				 NewFingerRots.index_01 = FRotator(0, -50, -12);
				 NewFingerRots.index_02 = FRotator(0, -62, 0);
				 NewFingerRots.index_03 = FRotator(0, -50, 0);
				 // Middle
				 NewFingerRots.middle_01 = FRotator(0, -60, -10);
				 NewFingerRots.middle_02 = FRotator(0, -70, 0);
				 NewFingerRots.middle_03 = FRotator(0, -60, 0);
				 // Ring
				 NewFingerRots.ring_01 = FRotator(0, -70, -10);
				 NewFingerRots.ring_02 = FRotator(0, -70, 0);
				 NewFingerRots.ring_03 = FRotator(0, -60, 0);
				 // Pinky
				 NewFingerRots.pinky_01 = FRotator(0, -90, -25);
				 NewFingerRots.pinky_02 = FRotator(0, -60, 0);
				 NewFingerRots.pinky_03 = FRotator(0, -60, 0);

				 NewObjRotation = FRotator(-36, 101, -24);
			 }
			 else {

				 // Object Location		 // ud  lr  bf
				 NewObjLocation = FVector(-7.85, 3, 1.55);
				 // Finger's Rotations
				 // Thumb
				 NewFingerRots.thumb_01 = FRotator(0, -20, 90);
				 NewFingerRots.thumb_02 = FRotator(0, -20, 0);
				 NewFingerRots.thumb_03 = FRotator(0, -10, 0);
				 // Index
				 NewFingerRots.index_01 = FRotator(0, -20, 0);
				 NewFingerRots.index_02 = FRotator(0, 0, 0);
				 NewFingerRots.index_03 = FRotator(12, 0, 0);
				 // Middle
				 NewFingerRots.middle_01 = FRotator(6, -15, 0);
				 NewFingerRots.middle_02 = FRotator(-5, -85, 0);
				 NewFingerRots.middle_03 = FRotator(0, -85, 0);
				 // Ring
				 NewFingerRots.ring_01 = FRotator(10, -40, 0);
				 NewFingerRots.ring_02 = FRotator(-5, -100, 0);
				 NewFingerRots.ring_03 = FRotator(0, -90, 0);
				 // Pinky
				 NewFingerRots.pinky_01 = FRotator(10, -80, 0);
				 NewFingerRots.pinky_02 = FRotator(-5, -100, 0);
				 NewFingerRots.pinky_03 = FRotator(0, -90, 0);

				 NewObjRotation = FRotator(-5, -5, -89);
			 }
		 }
		 else if (ObjectToGrasp->ActorHasTag(TEXT("Slice"))
					&& ObjectToGrasp->ActorHasTag(TEXT("Bread"))) {		// ********* Bread Slice *********

			UE_LOG(LogAvatarCharacter, Log, TEXT("Bread Slice Grasping"));

			// Object Location		 // ud  lr  bf
			NewObjLocation = FVector(-1, 4, 0);

			// Finger's Rotations
			// Thumb
			NewFingerRots.thumb_01 = FRotator(-63, 0, 30);
			NewFingerRots.thumb_02 = FRotator(0, -10, 0);
			NewFingerRots.thumb_03 = FRotator(0, -7, 0);
			// Index
			NewFingerRots.index_01 = FRotator(-3, 0, 0);
			NewFingerRots.index_02 = FRotator(0, -30, 0);
			NewFingerRots.index_03 = FRotator(0, -60, 0);
			// Middle
			NewFingerRots.middle_02 = FRotator(0, -50, 0);
			NewFingerRots.middle_03 = FRotator(0, -55, 0);
			// Ring
			NewFingerRots.ring_01 = FRotator(3, 0, 0);
			NewFingerRots.ring_02 = FRotator(0, -35, 0);
			NewFingerRots.ring_03 = FRotator(0, -60, 0);
			// Pinky
			NewFingerRots.pinky_01 = FRotator(5, 0, 0);
			NewFingerRots.pinky_02 = FRotator(0, -25, 10);
			NewFingerRots.pinky_03 = FRotator(0, -45, 0);

			// Object Rotation
			if (isGrasping_r) {      // p   y  r
				NewObjRotation = FRotator(90, 0, 0);
			}
			else {
				NewObjRotation = FRotator(90, 0, 0);
			}
		 }
		 else if (ObjectToGrasp->ActorHasTag(TEXT("Bowl"))) {			// ********* Bowl *********

			 // Attaching Rule
			 attachRules.RotationRule = EAttachmentRule::KeepRelative;

			 // Object Location
			 //NewObjLocation = FVector(-9.78, 9.9, -0.5); //changed for demo/yasmin
			 NewObjLocation = FVector(-3.5, 9.9, -0.5); // original

			 // Fingers' Rotations
			 // Thumb                         // p   y  r
			 NewFingerRots.thumb_01 = FRotator(-25, -5, 70);
			 NewFingerRots.thumb_02 = FRotator(5, -13, 0);
			 NewFingerRots.thumb_03 = FRotator(0, -30, 0);
			 // Index
			 NewFingerRots.index_01 = FRotator(-3, -7, 0);
			 NewFingerRots.index_02 = FRotator(0, -18, 0);
			 NewFingerRots.index_03 = FRotator(0, -20, 0);
			 // Middle
			 NewFingerRots.middle_01 = FRotator(0, -11, 0);
			 NewFingerRots.middle_02 = FRotator(0, -25, 0);
			 NewFingerRots.middle_03 = FRotator(0, -20, 0);
			 // Ring
			 NewFingerRots.ring_01 = FRotator(3, -16, 0);
			 NewFingerRots.ring_02 = FRotator(0, -15, 0);
			 NewFingerRots.ring_03 = FRotator(0, -20, 0);
			 // Pinky
			 NewFingerRots.pinky_01 = FRotator(5, -18, 0);
			 NewFingerRots.pinky_02 = FRotator(0, -20, 10);
			 NewFingerRots.pinky_03 = FRotator(0, -45, 0);

			 // Object Rotation
			 if (isGrasping_r) {
				 NewObjRotation = FRotator(-10, 0, 0);
			 }
			 else {
				 NewObjRotation = FRotator(170, 0, 0);
			 }
		 }
		 else if (ObjectToGrasp->ActorHasTag(TEXT("Jug"))) {		// ********** Milk Jug *********

			 if (isGrasping_r) {
				 // Object Rotation
				 NewObjRotation = FRotator(0, -30, 0);
			 }
			 else {
				 NewObjRotation = FRotator(-69.5, 0, 90);
			 }
		 }
		 else {		// ********** In General ***********

			 FVector BoxExtent;
			 UE_LOG(LogAvatarCharacter, Log, TEXT("Default Grasping"));

			 ObjectMesh->SetRelativeRotation(FRotator(0, 0, 0));
			 BoxExtent = ObjectMesh->Bounds.BoxExtent;
			 UE_LOG(LogAvatarCharacter, Log, TEXT("BoxExtent %f %f %f"), BoxExtent.X, BoxExtent.Y, BoxExtent.Z);
			 // Attaching Rule
			 attachRules.LocationRule = EAttachmentRule::KeepRelative;

			 // Put shortest axis along X and longest axis along Z
			 if ((BoxExtent.Y < BoxExtent.X) && (BoxExtent.Y < BoxExtent.Z)) {	// If Y is min
				 if (BoxExtent.X > BoxExtent.Z) {								  // If X is max
					 NewObjRotation.Pitch += 90;									    //    X'  = Z ; Y'  = Y ; Z'  = X ; 
					 NewObjRotation.Yaw += 90;										    //    X'' = Y'; Y'' = X'; Z'' = Z';  
					 NewObjLocation = FVector(0, BoxExtent.Z, 0);					    // => X'' = Y ; Y'' = Z ; Z'' = X ;
				 }
				 else {															  // If Z is max
					 NewObjRotation.Yaw += 90;										   // X' = Y; Y' = X; Z' = Z;
					 NewObjLocation = FVector(0, BoxExtent.X, 0);
				 }
			 }
			 else if ((BoxExtent.Z < BoxExtent.Y) && (BoxExtent.Z < BoxExtent.X)) {	// If Z is min
				 if (BoxExtent.Y > BoxExtent.X) {									  // If Y is max
					 NewObjRotation.Roll += 90;										    //    X'  = X ; Y'  = Z ; Z'  = Y ;
					 NewObjRotation.Yaw += 90;											    //    X'' = Y'; Y'' = X'; Z'' = Z';
					 NewObjLocation = FVector(0, BoxExtent.X, 0);						    // => X'' = Z ; Y'' = X ; Z'' = Y ;
				 }
				 else {																  // If X is max
					 NewObjRotation.Pitch += 90;										    // X' = Z; Y' = Y; Z' = X;
					 NewObjLocation = FVector(0, BoxExtent.Y, 0);
				 }
			 }
			 else if (BoxExtent.Y > BoxExtent.Z) {		// If X is min and Y is max
				 NewObjRotation.Roll += 90;					  // X' = X; Y' = Z; Z' = Y;
				 NewObjLocation = FVector(0, BoxExtent.Z, 0);

			 }
			 else { // If X in min and Z is max
				 NewObjLocation = FVector(0, BoxExtent.Y, 0);
			 }
			 NewObjLocation.Y += 2;
		 }

		 // Right or left Settings
		 if (isGrasping_r) {
			 socket = TEXT("hand_rSocket");
			 isGrasped_r = true;
			 graspedObject_r = ObjectToGrasp;
			 AnimationInstance->rightHandGraspingAlpha = 1.0;
			 AnimationInstance->RightHandFingerRots = NewFingerRots;
		 }
		 else {
			 socket = TEXT("hand_lSocket");
			 isGrasped_l = true;
			 graspedObject_l = ObjectToGrasp;
			 AnimationInstance->leftHandGraspingAlpha = 1.0;
			 AnimationInstance->LeftHandFingerRots = NewFingerRots;
			 NewObjLocation *= -1;
		 }

		 // Attach Object to hand socket
		 ObjectToGrasp->AttachToComponent(AvatarMesh, attachRules, socket);
		 
		 ObjectMesh->SetSimulatePhysics(false);
		 ObjectMesh->SetRelativeLocationAndRotation(NewObjLocation, NewObjRotation);

		 isGrasping_r = false;
		 isGrasping_l = false;

		 UE_LOG(LogAvatarCharacter, Log, TEXT("New Rot: (%f,%f,%f)"), NewObjRotation.Roll, NewObjRotation.Pitch, NewObjRotation.Yaw);
	 }
 }
#pragma optimize("", on)

 void AIAIAvatarCharacter::StopGraspObject() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("STOP Left TURNHAND"));
	 StopLeftHandReach();
 }

 void AIAIAvatarCharacter::StopGraspObject_r() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("STOP Right TURNHAND"));
	 StopRightHandReach();
 }

 void AIAIAvatarCharacter::DetachGraspObject() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Detach Left Handler called"));


	 //  https://answers.unrealengine.com/questions/549048/callfunctionbynamewitharguments-not-working.html?sort=oldest 
	 // You might have to allow BP calls explicitly
	 //FEditorScriptExecutionGuard ScriptGuard; 
	 FOutputDeviceNull ar;
	 const FString command = FString::Printf(TEXT("DetachFromCharacter"));
	 bool return_val = ObjectToGrasp->CallFunctionByNameWithArguments(*command, ar, NULL, true);
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Returned: %s "), return_val ? TEXT("TRUE") : TEXT("FALSE"));
	 isGrasped_l = false;

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 AnimationInstance->leftHandGraspingAlpha = 0;
 }

 void AIAIAvatarCharacter::DetachGraspObject_r() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Detach Right Handler called"));

	 //  https://answers.unrealengine.com/questions/549048/callfunctionbynamewitharguments-not-working.html?sort=oldest 
	 // You might have to allow BP calls explicitly
	 //FEditorScriptExecutionGuard ScriptGuard; 
	 FOutputDeviceNull ar;
	 const FString command = FString::Printf(TEXT("DetachFromCharacter"));
	 bool return_val = ObjectToGrasp_r->CallFunctionByNameWithArguments(*command, ar, NULL, true);
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Returned: %s "), return_val ? TEXT("TRUE") : TEXT("FALSE"));
	 isGrasped_r = false;
 }

 void AIAIAvatarCharacter::DetachGraspedObject_l() {

	 if (isGrasped_l) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Detach from left hand"));
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, "Detach from left hand", true, FVector2D(1.2, 1.2));

		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance != nullptr);

		 //AnimationInstance->leftHandGraspingAlpha = 0;
		 //AnimationInstance->resetLeftHandFingerRots();

		 graspedObject_l->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		 auto MeshCompt = Cast<UStaticMeshComponent>(graspedObject_l->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		 //MeshCompt->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		 MeshCompt->SetSimulatePhysics(true);
		 
		 isGrasped_l = false;
	 }
	 else {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Nothing to detach from left hand"));
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, "Warning: Nothing to detach from left hand", true, FVector2D(1.2, 1.2));
	 }
 }


 void AIAIAvatarCharacter::DetachGraspedObject_r() {
	 
	 if (isGrasped_r) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Detach from right hand"));
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, "Detach from right hand", true, FVector2D(1.2, 1.2));

		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance != nullptr);

		 //AnimationInstance->rightHandGraspingAlpha = 0;
		 //AnimationInstance->resetRightHandFingerRots();

		 graspedObject_r->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		 auto MeshCompt = Cast<UStaticMeshComponent>(graspedObject_r->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		 
		 //MeshCompt->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		 MeshCompt->SetSimulatePhysics(true);

		 isGrasped_r = false;
	 }
	 else {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Warning: Nothing to detach from right hand."));
		 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, "Warning: Nothing to detach from right hand.", true, FVector2D(1.2, 1.2));

	 }
 }

 void AIAIAvatarCharacter::StartRaiseHand() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Start raise left hand."));
	 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, "Start raise left hand.", true, FVector2D(1.2, 1.2));

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 AnimationInstance->LeftHandIKTargetPosition = HandTargetPosition;
	 //AnimationInstance->HandRotation = HandRotation;

	 IKEnableActive = true;
	 IKEnableTickDirection = 1;
 }
 
 void AIAIAvatarCharacter::StartRaiseHand_r() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Start raising right hand."));
	 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, "Start raising right hand.", true, FVector2D(1.2, 1.2));

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 AnimationInstance->RightHandIKTargetPosition = RightHandTargetPosition;
	 //AnimationInstance->RightHandRotation = RightHandRotation;

	 IKEnableActive_r = true;
	 IKEnableTickDirection_r = 1;
 }

 void AIAIAvatarCharacter::StopRaiseHand() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Stop raising left hand."));
	 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, "Stop raising left hand.", true, FVector2D(1.2, 1.2));

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 IKEnableActive = true;
	 IKEnableTickDirection = -1;
 }

 void AIAIAvatarCharacter::StopRaiseHand_r() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Stop raising right hand."));
	 GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, "Stop raising right hand.", true, FVector2D(1.2, 1.2));

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 IKEnableActive_r = true;
	 IKEnableTickDirection_r = -1;
 }

 void AIAIAvatarCharacter::SimpleMoveToTargetPoint() {
	 UE_LOG(LogAvatarCharacter, Log, TEXT("AutoMove"));
	 UNavigationSystemV1 *NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	 check(NavigationSystem != nullptr);
	 //NavigationSystem->SimpleMoveToLocation(GetController(), FVector(-740, 940, 200));
	 UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), TargetPoint); // TODO comment back in!

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);
 }

 void AIAIAvatarCharacter::MoveTo(FVector PositionInWorld) {
	 if (!ControlledByAI()) {
		 UE_LOG(LogAvatarCharacter, Error, TEXT("AIAIAvatarCharacter::MoveTo can only be used on an AI Controlled Avatar. Exiting..."));
		 return;
	 }
	 UE_LOG(LogAvatarCharacter, Log, TEXT("[%s] MoveTo %f %f %f"), *GetName(), PositionInWorld.X , PositionInWorld.Y, PositionInWorld.Z);

	 UNavigationSystemV1 *NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	 check(NavigationSystem != nullptr);
	 ////NavigationSystem->SimpleMoveToLocation(GetController(), PositionInWorld);

	 check(GetController());
	 AAIController* x = Cast<AAIController>(GetController());
	 check(x);

	 FOutputDeviceNull ar;
	 const FString command = FString::Printf(TEXT("MoveTo %f %f %f"), PositionInWorld.X, PositionInWorld.Y, PositionInWorld.Z);
	 bool return_val = x->CallFunctionByNameWithArguments(*command, ar, NULL, true);
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Returned: %s "), return_val ? TEXT("TRUE") : TEXT("FALSE"));

	 /*
	 EPathFollowingRequestResult::Type MoveToLocationResult;
	 MoveToLocationResult = x->MoveToLocation(PositionInWorld,  // const FVector & Dest,
		 2.0f, // float AcceptanceRadius,
		 true, // bool bStopOnOverlap,
		 true, // bool bUsePathfinding,
		 true, // bool bProjectDestinationToNavigation,
		 true, // bool bCanStrafe,
		 TSubclassOf< UNavigationQueryFilter >(),
		 true // bool bAllowPartialPath
		 );

	 switch (MoveToLocationResult) {
	 case EPathFollowingRequestResult::Type::Failed:
		 UE_LOG(LogAvatarCharacter, Warning, TEXT("MoveToLocation failed"));
		 break;
	 case EPathFollowingRequestResult::Type::AlreadyAtGoal:
		 UE_LOG(LogAvatarCharacter, Warning, TEXT("MoveToLocation not executed: AlreadyAtGoal"));
		 break;
	 case EPathFollowingRequestResult::Type::RequestSuccessful:
		 UE_LOG(LogAvatarCharacter, Warning, TEXT("MoveToLocation succeeded"));
		 break;
	 }*/
 }

 void AIAIAvatarCharacter::StartPathFollowing(FString Path) {

	 /*if (!ControlledByAI()) {
		 UE_LOG(LogAvatarCharacter, Error, TEXT("Path following can only be used on an AI Controlled Avatar. Exiting..."));
		 return;
	 }*/
	 UE_LOG(LogAvatarCharacter, Log, TEXT("[%s] Follow Spline %s"), *GetName(), *Path);

	 UNavigationSystemV1 *NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	 check(NavigationSystem != nullptr);
	 ////NavigationSystem->SimpleMoveToLocation(GetController(), PositionInWorld);

	 check(GetController());
	 AAIController* x = Cast<AAIController>(GetController());
	 check(x);

	 FOutputDeviceNull ar;
	 const FString command = FString::Printf(TEXT("StartSpline %s"), *Path);
	 bool return_val = x->CallFunctionByNameWithArguments(*command, ar, NULL, true);
	 UE_LOG(LogAvatarCharacter, Log, TEXT("Returned: %s "), return_val ? TEXT("TRUE") : TEXT("FALSE"));

 }

 void AIAIAvatarCharacter::TurnTo(FString mode, float angle) {

	 // Get Animation
	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 if (mode.Equals("camera")) {
		 AnimationInstance->DesiredActorRotation = GetFollowCamera()->GetComponentRotation().Yaw;
		 AnimationInstance->EnableActorRotation = true;
	 }
	 else if (mode.Equals("angle")) {
		 AnimationInstance->DesiredActorRotation = angle;
		 AnimationInstance->EnableActorRotation = true;
	 }
 }

void AIAIAvatarCharacter::BeginPlay() {
	Super::BeginPlay();
	
	 // Decide if this is a Player or AI controlled instance
	check(ControlledByAI() || ControlledByPlayer());

	 if (ControlledByAI()) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Actor %s is AI controlled"), *GetName());
	 }
	 else {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Actor %s is Player controlled"), *GetName());
	 }
	 
	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 CurrentAnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());

	 AnimationInstance->LeftHandIKTargetPosition = HandTargetPosition;
	 AnimationInstance->RightHandIKTargetPosition = RightHandTargetPosition;
	 AnimationInstance->Spine1Rotation = Spine1Rotation;
	 AnimationInstance->Spine2Rotation = Spine2Rotation;
	 AnimationInstance->HipRotation    = HipRotation;
	 AnimationInstance->HandRotation = HandRotation;
	 AnimationInstance->RightHandRotation = RightHandRotation;

	 // HeadPIDController.P = 1.0f;
	 // HeadPIDController.I = 0.0f;
	 // HeadPIDController.D = 0.0f;
	 // HeadPIDController.MaxOutAbs = 2.f;
	 //
	 // HeadPIDController.Init();

}



void AIAIAvatarCharacter::SetAbsoluteActorRotationWithTimeline(FRotator rot) {
	// Set up the rotation 
	if (ActorRotationCurveFloat)
	{
		InitialActorRotation = GetActorRotation();
		ActorRotationTarget = rot;

		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleActorRotationTimelineProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		ActorRotationTimeline.AddInterpFloat(ActorRotationCurveFloat, ProgressFunction);
		ActorRotationTimeline.SetLooping(false);

		//Starting the timeline...
		ActorRotationTimeline.PlayFromStart();
	}
	else 
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("Can't execute SetAbsoluteActorRotationWithTimeline with an ActorRotationCurveFloat. Please set a curve in the Avatar Settings."));
	}

}

void AIAIAvatarCharacter::Feed(FString PersonName) {

	// Local variables
	TMap<FString, FHitResult> MyUniqueHits;
	FVector TargetLocation;
	FVector MidPoint;
	FRotator PersonRotation;

	FTimerHandle HandIKTimeHandle_target;
	FTimerHandle HandIKTimeHandle_mid;
	FTimerHandle HandIKTimeHandle_end;
	FTimerHandle SpineTimeHandle_end;
	FTimerHandle HandRotTimeHandle_end;

	FTimerDelegate HandIKSetDelegate_target;
	FTimerDelegate HandIKSetDelegate_mid;
	FTimerDelegate HandIKSetDelegate_end;
	FTimerDelegate SpineDelegate_end;
	FTimerDelegate HandRotDelegate_end;

	MyUniqueHits = ListObjects();

	// Verify list hasn't changed
	if (MyUniqueHits.FindRef(PersonName).GetActor() == NULL) {
		UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Character \"%s\" not found!"), *PersonName);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: Character \"%s\" not found!"), *PersonName), true, FVector2D(1.7, 1.7));
	}
	else {
		
		ACharacter *Person = Cast<ACharacter>(MyUniqueHits.FindRef(PersonName).GetActor());

		if (Person == NULL) {
			UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Actor \"%s\" is not a character!"), *PersonName);
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: Actor \"%s\" is not a character!"), *PersonName), true, FVector2D(1.7, 1.7));
		}
		else {

			UE_LOG(LogAvatarCharacter, Log, TEXT("Feeding: %s "), *PersonName);
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, FString::Printf(TEXT("Feeding: %s"), *PersonName), true, FVector2D(1.2, 1.2));

			// Person's Local Space
			TargetLocation = Person->GetMesh()->GetBoneLocation(TEXT("jaw_end"), EBoneSpaces::ComponentSpace);
			TargetLocation += FVector(-6, 19, 0);
			MidPoint = TargetLocation + FVector(0,20,0);

			// World Space
			TargetLocation = Person->GetMesh()->GetComponentTransform().TransformPosition(TargetLocation);
			MidPoint = Person->GetMesh()->GetComponentTransform().TransformPosition(MidPoint);

			// My Character's Local Space
			TargetLocation = GetMesh()->GetComponentTransform().InverseTransformPosition(TargetLocation);
			MidPoint = GetMesh()->GetComponentTransform().InverseTransformPosition(MidPoint);
			PersonRotation = Person->GetViewRotation();

			HandIKSetDelegate_target = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, TargetLocation);
			HandIKSetDelegate_mid = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, MidPoint);
			HandIKSetDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::InterpolateRightHandIKTo, FVector(-15,15,105));
			SpineDelegate_end = FTimerDelegate::CreateUObject(this, &AIAIAvatarCharacter::StartSpineDisablement);

			// 0s) Move hand to approach location
			InterpolateRightHandIKTo(MidPoint);

			// 0s) Star spine rotation
			StartSpineEnablement(FRotator(0,25,0));

			// 1s) Move hand to target location
			GetWorldTimerManager().SetTimer(HandIKTimeHandle_target, HandIKSetDelegate_target, 5, false, 1);

			// 3.5s) Move hand back to mid point
			GetWorldTimerManager().SetTimer(HandIKTimeHandle_mid, HandIKSetDelegate_mid, 5, false, 3.5);

			// 4s) Move hand to holding position
			GetWorldTimerManager().SetTimer(HandIKTimeHandle_end, HandIKSetDelegate_end, 5, false, 4.5);

			// 4s) Stop Spine rotation
			GetWorldTimerManager().SetTimer(SpineTimeHandle_end, SpineDelegate_end, 5, false, 4.5);
		}
	}
}

void AIAIAvatarCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	 CurrentAnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(CurrentAnimationInstance != nullptr);

	 //Advancing the actor timeline in every tick
	 ActorRotationTimeline.TickTimeline(DeltaTime);
	 //LeftHandIKTimeline.TickTimeline(DeltaTime);
	 SpineInterpolation.Timeline.TickTimeline(DeltaTime);
	 SpineBoneInterpolation.Timeline.TickTimeline(DeltaTime);
	 LeftHandRotationAlphaInterpolation.Timeline.TickTimeline(DeltaTime);
	 LeftHandRotationBoneInterpolation.Timeline.TickTimeline(DeltaTime);
	 LeftHandIKAlphaInterpolation.Timeline.TickTimeline(DeltaTime);
	 LeftHandIKBoneInterpolation.Timeline.TickTimeline(DeltaTime);
	 RightHandRotationAlphaInterpolation.Timeline.TickTimeline(DeltaTime);
	 RightHandRotationBoneInterpolation.Timeline.TickTimeline(DeltaTime);
	 RightHandIKAlphaInterpolation.Timeline.TickTimeline(DeltaTime);
	 RightHandIKBoneInterpolation.Timeline.TickTimeline(DeltaTime);
	 LeftFingerIKAlphaInterpolation.Timeline.TickTimeline(DeltaTime);
	 LeftFingerIKBoneInterpolation.Timeline.TickTimeline(DeltaTime);
	 RightFingerIKAlphaInterpolation.Timeline.TickTimeline(DeltaTime);
	 RightFingerIKBoneInterpolation.Timeline.TickTimeline(DeltaTime);

	 //Handle IK Enablement (grasping and friends)
	
	 if (IKEnableActive) {
		 HandleIKEnablement_l(DeltaTime);
	 }

	 if (IKEnableActive_r) {
		 HandleIKEnablement_r(DeltaTime);
	 }


	 {
		 FRotator CurrentHeadRotation = CurrentAnimationInstance->HeadRotation;

//		 const FRotator HeadRotationError = GoalHeadRotation - CurrentHeadRotation;
		 //UE_LOG(LogTemp, Error, TEXT("Rotation State. Anim %s , Current %s , Goal %s, Error: %s"),
		 //	*CurrentAnimationInstance->HeadRotation.ToString(),
		 //	*CurrentHeadRotation.ToString(),
		 //	*GoalHeadRotation.ToString(),
		 //	*HeadRotationError.ToString());


		 // Convert to vector representation to comply with the PID Controller interface
	//	 const FVector HeadRotationErrorVector(HeadRotationError.Pitch, HeadRotationError.Yaw, HeadRotationError.Roll);

		 // Get the magnitude of the Vector and decide if we're close enough to the desired Head Pose
		 /*if ( HeadRotationErrorVector.Size() <= HeadRotationErrorThreshold) {
			 return;
		 }*/

	//	 UE_LOG(LogAvatarCharacter, Log, TEXT("  Difference between current head and goal: %s = %f"), *(HeadRotationErrorVector.ToString()), HeadRotationErrorVector.Size());

		// const FVector PIDOutputVector = HeadPIDController.Update(HeadRotationErrorVector, DeltaTime);
		// UE_LOG(LogAvatarCharacter, Log, TEXT("  PID Output is: %s"), *(PIDOutputVector.ToString()));

		 // const FRotator PIDOutputRotator = FRotator(PIDOutputVector.X, PIDOutputVector.Y, PIDOutputVector.Z);

		 // CurrentAnimationInstance->HeadRotation = PIDOutputRotator + CurrentHeadRotation;
		 // UE_LOG(LogAvatarCharacter, Log, TEXT("  New Head Rotation shall be: %s"), *(CurrentAnimationInstance->HeadRotation.ToString()));
	 }
	
}

void AIAIAvatarCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AIAIAvatarCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

 void AIAIAvatarCharacter::HandleIKEnablement_l(float DeltaTime) {

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 const float Interp_Speed = 6.0f;

	 if (IKEnableTickDirection == -1 && AnimationInstance->LeftHandIKAlpha <= 0.0f) {
		 IKEnableActive = false;
		 return;
	 }
	 if (IKEnableTickDirection == 1 && AnimationInstance->LeftHandIKAlpha >= 1.0f) {
		 IKEnableActive = false;
		 return;
	 }

	 if (IKEnableTickDirection == 1) {
		 AnimationInstance->LeftHandIKAlpha = FMath::FInterpTo(AnimationInstance->LeftHandIKAlpha, 1.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->HandRotationAlpha = FMath::FInterpTo(AnimationInstance->HandRotationAlpha, 1.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->SpineRotationAlpha = FMath::FInterpTo(AnimationInstance->SpineRotationAlpha, 1.0f, DeltaTime, Interp_Speed);
	 }
	 else {
		 AnimationInstance->LeftHandIKAlpha = FMath::FInterpTo(AnimationInstance->LeftHandIKAlpha, 0.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->HandRotationAlpha = FMath::FInterpTo(AnimationInstance->HandRotationAlpha, 0.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->SpineRotationAlpha = FMath::FInterpTo(AnimationInstance->SpineRotationAlpha, 0.0f, DeltaTime, Interp_Speed);
	 }
 }

 void AIAIAvatarCharacter::HandleIKEnablement_r(float DeltaTime) {

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance != nullptr);

	 const float Interp_Speed = 6.0f;

	 if (IKEnableTickDirection_r == -1 && AnimationInstance->RightHandIKAlpha <= 0.0f) {
		 IKEnableActive_r = false;
		 return;
	 }
	 if (IKEnableTickDirection_r == 1 && AnimationInstance->RightHandIKAlpha >= 1.0f) {
		 IKEnableActive_r = false;
		 return;
	 }

    if (IKEnableTickDirection_r == 1) {
		 AnimationInstance->RightHandIKAlpha = FMath::FInterpTo(AnimationInstance->RightHandIKAlpha, 1.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->RightHandRotationAlpha = FMath::FInterpTo(AnimationInstance->RightHandRotationAlpha, 1.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->SpineRotationAlpha = FMath::FInterpTo(AnimationInstance->SpineRotationAlpha, 1.0f, DeltaTime, Interp_Speed);
	 }
	 else {
		 AnimationInstance->RightHandIKAlpha = FMath::FInterpTo(AnimationInstance->RightHandIKAlpha, 0.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->RightHandRotationAlpha = FMath::FInterpTo(AnimationInstance->RightHandRotationAlpha, 0.0f, DeltaTime, Interp_Speed);
		 AnimationInstance->SpineRotationAlpha = FMath::FInterpTo(AnimationInstance->SpineRotationAlpha, 0.0f, DeltaTime, Interp_Speed);
	 }
 }

 void AIAIAvatarCharacter::HandleActorRotationTimelineProgress(float Value)
 {
	 FRotator NewRotation = FMath::Lerp(InitialActorRotation, ActorRotationTarget, Value);
	 SetActorRotation(FQuat(NewRotation));
 }

 void AIAIAvatarCharacter::HandleSpineAlphaInterpolationProgress(float Value) {
	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance);
	 AnimationInstance->SpineRotationAlpha = Value;
	 //UE_LOG(LogTemp, Error, TEXT("Interpol active %f"), Value);
 }

 void AIAIAvatarCharacter::HandleSpineBoneInterpolationProgress(float Value) {
	 FRotator New = FMath::Lerp(SpineBoneInterpolation.InitialValue, SpineBoneInterpolation.TargetValue, Value);

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	 check(AnimationInstance);

	 AnimationInstance->Spine1Rotation = New;

	 //UE_LOG(LogTemp, Error, TEXT("Bone Interpol active %f"), Value);
 }

 void AIAIAvatarCharacter::StartSpineEnablement(FRotator Spine1, FRotator Spine2, FRotator Hip) {
	 if (SpineCurveFloat)
	 {
		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance);

		 // Setting the target of the bone modification
		 AnimationInstance->Spine1Rotation = Spine1;
		 AnimationInstance->Spine2Rotation = Spine2;
		 AnimationInstance->HipRotation = Hip;

		 // Initializing the actual timeline / smooth transition
		 SpineInterpolation.InitialValue = AnimationInstance->SpineRotationAlpha;
		 SpineInterpolation.TargetValue = 1;

		 UE_LOG(LogAvatarCharacter, Log, TEXT("Spine Interpolation from %f to %f"), SpineInterpolation.InitialValue, SpineInterpolation.TargetValue);
		 
		 SpineInterpolation.ProgressFunction.BindUFunction(this, FName("HandleSpineAlphaInterpolationProgress"));

		 //Setting up the loop status and the function that is going to fire when the timeline ticks
		 SpineInterpolation.Timeline.AddInterpFloat(SpineCurveFloat, SpineInterpolation.ProgressFunction);
		 SpineInterpolation.Timeline.SetLooping(false);

		 //Starting the timeline...
		 SpineInterpolation.Timeline.PlayFromStart();


	 }
	 else
	 {
		 UE_LOG(LogAvatarCharacter, Error, TEXT("Can't execute StartSpineEnablement without an SpineCurveFloat"));
	 }
 }
 void AIAIAvatarCharacter::InterpolateSpineTo(FRotator NewRot) {
	 if (SpineCurveFloat)
	 {
		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance);
		 UE_LOG(LogAvatarCharacter, Error, TEXT("InterpolateSpineTo"));
		 SpineBoneInterpolation.InitialValue	= AnimationInstance->Spine1Rotation;
		 SpineBoneInterpolation.TargetValue		= NewRot;
		 SpineBoneInterpolation.ProgressFunction.BindUFunction(this, FName("HandleSpineBoneInterpolationProgress"));

		 SpineBoneInterpolation.Timeline.AddInterpFloat(SpineCurveFloat, SpineBoneInterpolation.ProgressFunction);
		 SpineBoneInterpolation.Timeline.SetLooping(false);
		 SpineBoneInterpolation.Timeline.PlayFromStart();
	 }
	 else
	 {
		 UE_LOG(LogAvatarCharacter, Error, TEXT("Can't execute InterpolateSpineTo without an SpineCurveFloat"));
	 }
 }

 void AIAIAvatarCharacter::StartSpineDisablement() {
	 if (SpineCurveFloat)
	 {
		 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		 check(AnimationInstance);

		 // Setting the target of the bone modification
		 
		 // Initializing the actual timeline / smooth transition
		 SpineInterpolation.InitialValue = AnimationInstance->SpineRotationAlpha;
		 SpineInterpolation.TargetValue = 0;

		 UE_LOG(LogAvatarCharacter, Log, TEXT("Spine Interpolation from %f to %f"), SpineInterpolation.InitialValue, SpineInterpolation.TargetValue);

		 SpineInterpolation.ProgressFunction.BindUFunction(this, FName("HandleSpineAlphaInterpolationProgress"));

		 //Setting up the loop status and the function that is going to fire when the timeline ticks
		 SpineInterpolation.Timeline.AddInterpFloat(SpineCurveFloat, SpineInterpolation.ProgressFunction);
		 SpineInterpolation.Timeline.SetLooping(false);

		 //Starting the timeline...
		 SpineInterpolation.Timeline.ReverseFromEnd();


	 }
	 else
	 {
		 UE_LOG(LogAvatarCharacter, Error, TEXT("Can't execute StartSpineEnablement without an SpineCurveFloat"));
	 }
 }

// Right Hand Rotation Interpolation handling
void AIAIAvatarCharacter::HandleLeftHandRotationAlphaInterpolationProgress(float Value) {
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);
	AnimationInstance->HandRotationAlpha = Value;
	//UE_LOG(LogTemp, Error, TEXT("LH: Rot Interpol active %f"), Value);
}

void AIAIAvatarCharacter::HandleLeftHandRotationBoneInterpolationProgress(float Value) {
	FRotator New = FMath::Lerp(LeftHandRotationBoneInterpolation.InitialValue, LeftHandRotationBoneInterpolation.TargetValue, Value);

	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);

	AnimationInstance->HandRotation = New;

	//UE_LOG(LogTemp, Error, TEXT("LH: Rot Bone Interpol active %f"), Value);
}

void AIAIAvatarCharacter::StartLeftHandRotationEnablement(FRotator NewRot) {
	if (HandRotationCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Setting the target of the bone modification
		AnimationInstance->HandRotation = NewRot;

		// Initializing the actual timeline / smooth transition
		LeftHandRotationAlphaInterpolation.InitialValue = AnimationInstance->HandRotationAlpha;
		LeftHandRotationAlphaInterpolation.TargetValue = 1;

		UE_LOG(LogAvatarCharacter, Log, TEXT("LH: Rot Interpolation from %f to %f"), LeftHandRotationAlphaInterpolation.InitialValue, 
			LeftHandRotationAlphaInterpolation.TargetValue);

		LeftHandRotationAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftHandRotationAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		LeftHandRotationAlphaInterpolation.Timeline.AddInterpFloat(HandRotationCurveFloat, LeftHandRotationAlphaInterpolation.ProgressFunction);
		LeftHandRotationAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		LeftHandRotationAlphaInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute StartLeftHandRotationEnablement without a HandRotationCurveFloat"));
	}

}

void AIAIAvatarCharacter::InterpolateLeftHandRotationTo(FRotator NewRot) {
	if (HandRotationCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: InterpolateLeftHandRotationTo"));
		LeftHandRotationBoneInterpolation.InitialValue = AnimationInstance->HandRotation;
		LeftHandRotationBoneInterpolation.TargetValue = NewRot;
		LeftHandRotationBoneInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftHandRotationBoneInterpolationProgress"));

		LeftHandRotationBoneInterpolation.Timeline.AddInterpFloat(HandRotationCurveFloat,
			LeftHandRotationBoneInterpolation.ProgressFunction);
		LeftHandRotationBoneInterpolation.Timeline.SetLooping(false);

		LeftHandRotationBoneInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute InterpolateSpineTo without a HandRotationCurveFloat"));
	}
}

void AIAIAvatarCharacter::StartLeftHandRotationDisablement() {
	if (HandRotationCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Initializing the actual timeline / smooth transition
		LeftHandRotationAlphaInterpolation.InitialValue = AnimationInstance->HandRotationAlpha;
		LeftHandRotationAlphaInterpolation.TargetValue = 0;

		UE_LOG(LogAvatarCharacter, Log, TEXT("LH: Rot Interpolation from %f to %f"), LeftHandRotationAlphaInterpolation.InitialValue, 
			LeftHandRotationAlphaInterpolation.TargetValue);

		LeftHandRotationAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftHandRotationAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		LeftHandRotationAlphaInterpolation.Timeline.AddInterpFloat(HandRotationCurveFloat, LeftHandRotationAlphaInterpolation.ProgressFunction);
		LeftHandRotationAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		LeftHandRotationAlphaInterpolation.Timeline.ReverseFromEnd();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute StartLeftHandRotationDisablement without a HandRotationCurveFloat"));
	}
}

// Right Hand Rotation Interpolation handling
void AIAIAvatarCharacter::HandleRightHandRotationAlphaInterpolationProgress(float Value) {
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);
	AnimationInstance->RightHandRotationAlpha = Value;
	//UE_LOG(LogTemp, Error, TEXT("RH: Rot Interpol active %f"), Value);
}
void AIAIAvatarCharacter::HandleRightHandRotationBoneInterpolationProgress(float Value) {
	FRotator New = FMath::Lerp(RightHandRotationBoneInterpolation.InitialValue, RightHandRotationBoneInterpolation.TargetValue, Value);

	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);

	AnimationInstance->RightHandRotation = New;

	//UE_LOG(LogTemp, Error, TEXT("RH: Rot Bone Interpol active %f"), Value);
}
void AIAIAvatarCharacter::StartRightHandRotationEnablement(FRotator NewRot) {
	if (HandRotationCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Setting the target of the bone modification
		AnimationInstance->RightHandRotation = NewRot;

		// Initializing the actual timeline / smooth transition
		RightHandRotationAlphaInterpolation.InitialValue = AnimationInstance->RightHandRotationAlpha;
		RightHandRotationAlphaInterpolation.TargetValue = 1;

		UE_LOG(LogAvatarCharacter, Log, TEXT("RH: Rot Interpolation from %f to %f"), RightHandRotationAlphaInterpolation.InitialValue,
			RightHandRotationAlphaInterpolation.TargetValue);

		RightHandRotationAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightHandRotationAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		RightHandRotationAlphaInterpolation.Timeline.AddInterpFloat(HandRotationCurveFloat, RightHandRotationAlphaInterpolation.ProgressFunction);
		RightHandRotationAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		RightHandRotationAlphaInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute StartRightHandRotationEnablement without a HandRotationCurveFloat"));
	}

}
void AIAIAvatarCharacter::InterpolateRightHandRotationTo(FRotator NewRot) {
	if (HandRotationCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: InterpolateRightHandRotationTo"));
		RightHandRotationBoneInterpolation.InitialValue = AnimationInstance->HandRotation;
		RightHandRotationBoneInterpolation.TargetValue = NewRot;
		RightHandRotationBoneInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightHandRotationBoneInterpolationProgress"));

		RightHandRotationBoneInterpolation.Timeline.AddInterpFloat(HandRotationCurveFloat,
		RightHandRotationBoneInterpolation.ProgressFunction);
		RightHandRotationBoneInterpolation.Timeline.SetLooping(false);

		RightHandRotationBoneInterpolation.Timeline.PlayFromStart();
	}                                                                                                                  
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute InterpolateSpineTo without a HandRotationCurveFloat"));
	}
}
void AIAIAvatarCharacter::StartRightHandRotationDisablement() {
	if (HandRotationCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Initializing the actual timeline / smooth transition
		RightHandRotationAlphaInterpolation.InitialValue = AnimationInstance->RightHandRotationAlpha;
		RightHandRotationAlphaInterpolation.TargetValue = 0;

		UE_LOG(LogAvatarCharacter, Log, TEXT("RH: Rot Interpolation from %f to %f"), RightHandRotationAlphaInterpolation.InitialValue,
			RightHandRotationAlphaInterpolation.TargetValue);

		RightHandRotationAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightHandRotationAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		RightHandRotationAlphaInterpolation.Timeline.AddInterpFloat(HandRotationCurveFloat, RightHandRotationAlphaInterpolation.ProgressFunction);
		RightHandRotationAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		RightHandRotationAlphaInterpolation.Timeline.ReverseFromEnd();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute StartRightHandRotationDisablement without a HandRotationCurveFloat"));
	}
}

// Left Hand IK interpolation handling
void AIAIAvatarCharacter::HandleLeftHandIKAlphaInterpolationProgress(float Value) 
{
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);
	AnimationInstance->LeftHandIKAlpha = Value;
	//UE_LOG(LogTemp, Error, TEXT("LH: Interpol active %f"), Value);
}

void AIAIAvatarCharacter::HandleLeftHandIKBoneInterpolationProgress(float Value)
{
	FVector New = FMath::Lerp(LeftHandIKBoneInterpolation.InitialValue, LeftHandIKBoneInterpolation.TargetValue, Value);

	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);

	AnimationInstance->LeftHandIKTargetPosition = New;

	//UE_LOG(LogTemp, Error, TEXT("LH: Bone Interpol active %f"), Value);
}

void AIAIAvatarCharacter::StartLeftHandIKEnablement(FVector NewVec)
{
	if (LeftHandIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Setting the target of the bone modification
		AnimationInstance->LeftHandIKTargetPosition = NewVec;

		// Initializing the actual timeline / smooth transition
		LeftHandIKAlphaInterpolation.InitialValue = AnimationInstance->LeftHandIKAlpha;
		LeftHandIKAlphaInterpolation.TargetValue = 1;

		UE_LOG(LogAvatarCharacter, Log, TEXT("LH: Interpolation from %f to %f"), LeftHandIKAlphaInterpolation.InitialValue,
			LeftHandIKAlphaInterpolation.TargetValue);

		LeftHandIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftHandIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		LeftHandIKAlphaInterpolation.Timeline.AddInterpFloat(LeftHandIKCurveFloat, LeftHandIKAlphaInterpolation.ProgressFunction);
		LeftHandIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		LeftHandIKAlphaInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute StartLeftHandIKEnablement without a LeftHandIKCurveFloat"));
	}
}
void AIAIAvatarCharacter::InterpolateLeftHandIKTo(FVector NewVec)
{
	if (LeftHandIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: InterpolateLeftHandIKTo (%f,%f,%f)"), NewVec.X, NewVec.Y, NewVec.Z);

		LeftHandIKBoneInterpolation.InitialValue = AnimationInstance->LeftHandIKTargetPosition;
		LeftHandIKBoneInterpolation.TargetValue = NewVec;
		LeftHandIKBoneInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftHandIKBoneInterpolationProgress"));

		LeftHandIKBoneInterpolation.Timeline.AddInterpFloat(LeftHandIKCurveFloat,
		LeftHandIKBoneInterpolation.ProgressFunction);
		LeftHandIKBoneInterpolation.Timeline.SetLooping(false);

		LeftHandIKBoneInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute InterpolateLeftHandIKTo without a LeftHandIKCurveFloat"));
	}
}
void AIAIAvatarCharacter::StartLeftHandIKDisablement()
{
	if (LeftHandIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Initializing the actual timeline / smooth transition
		LeftHandIKAlphaInterpolation.InitialValue = AnimationInstance->LeftHandIKAlpha;
		LeftHandIKAlphaInterpolation.TargetValue = 0;

		UE_LOG(LogAvatarCharacter, Log, TEXT("LH: Interpolation from %f to %f"), LeftHandIKAlphaInterpolation.InitialValue,
		LeftHandIKAlphaInterpolation.TargetValue);

		LeftHandIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftHandIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		LeftHandIKAlphaInterpolation.Timeline.AddInterpFloat(LeftHandIKCurveFloat, LeftHandIKAlphaInterpolation.ProgressFunction);
		LeftHandIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		LeftHandIKAlphaInterpolation.Timeline.ReverseFromEnd();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute StartLeftHandIKDisablement without a LeftHandIKCurveFloat"));
	}
}

// Right Hand IK interpolation Handling
void AIAIAvatarCharacter::HandleRightHandIKAlphaInterpolationProgress(float Value)
{
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);
	AnimationInstance->RightHandIKAlpha = Value;
	//UE_LOG(LogTemp, Error, TEXT("RH: Interpol active %f"), Value);
}

void AIAIAvatarCharacter::HandleRightHandIKBoneInterpolationProgress(float Value)
{
	FVector New = FMath::Lerp(RightHandIKBoneInterpolation.InitialValue, RightHandIKBoneInterpolation.TargetValue, Value);

	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);

	AnimationInstance->RightHandIKTargetPosition = New;

	//UE_LOG(LogTemp, Error, TEXT("RH: Bone Interpol active %f"), Value);
}

void AIAIAvatarCharacter::StartRightHandIKEnablement(FVector NewVec)
{
	if (RightHandIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Setting the target of the bone modification
		AnimationInstance->RightHandIKTargetPosition = NewVec;

		// Initializing the actual timeline / smooth transition
		RightHandIKAlphaInterpolation.InitialValue = AnimationInstance->RightHandIKAlpha;
		RightHandIKAlphaInterpolation.TargetValue = 1;

		UE_LOG(LogAvatarCharacter, Log, TEXT("RH: Interpolation from %f to %f"), RightHandIKAlphaInterpolation.InitialValue,
			RightHandIKAlphaInterpolation.TargetValue);

		RightHandIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightHandIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		RightHandIKAlphaInterpolation.Timeline.AddInterpFloat(RightHandIKCurveFloat, RightHandIKAlphaInterpolation.ProgressFunction);
		RightHandIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		RightHandIKAlphaInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute StartRightHandIKEnablement without a RightHandIKCurveFloat"));
	}
}
void AIAIAvatarCharacter::InterpolateRightHandIKTo(FVector NewVec)
{
	if (RightHandIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);
		UE_LOG(LogAvatarCharacter, Warning, TEXT("RH: InterpolateRightHandIKTo (%f,%f,%f)"),NewVec.X,NewVec.Y,NewVec.Z);

		RightHandIKBoneInterpolation.InitialValue = AnimationInstance->RightHandIKTargetPosition;
		RightHandIKBoneInterpolation.TargetValue = NewVec;
		RightHandIKBoneInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightHandIKBoneInterpolationProgress"));

		RightHandIKBoneInterpolation.Timeline.AddInterpFloat(RightHandIKCurveFloat,
			RightHandIKBoneInterpolation.ProgressFunction);
		RightHandIKBoneInterpolation.Timeline.SetLooping(false);

		RightHandIKBoneInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute InterpolateRightHandIKTo without a RightHandIKCurveFloat"));
	}
}
void AIAIAvatarCharacter::StartRightHandIKDisablement()
{
	if (RightHandIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Initializing the actual timeline / smooth transition
		RightHandIKAlphaInterpolation.InitialValue = AnimationInstance->RightHandIKAlpha;
		RightHandIKAlphaInterpolation.TargetValue = 0;

		UE_LOG(LogAvatarCharacter, Log, TEXT("RH: Interpolation from %f to %f"), RightHandIKAlphaInterpolation.InitialValue,
			RightHandIKAlphaInterpolation.TargetValue);

		RightHandIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightHandIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		RightHandIKAlphaInterpolation.Timeline.AddInterpFloat(RightHandIKCurveFloat, RightHandIKAlphaInterpolation.ProgressFunction);
		RightHandIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		RightHandIKAlphaInterpolation.Timeline.ReverseFromEnd();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute StartRightHandIKDisablement without a RightHandIKCurveFloat"));
	}
}


// Left Finger IK interpolation handling
void AIAIAvatarCharacter::HandleLeftFingerIKAlphaInterpolationProgress(float Value)
{
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);
	AnimationInstance->LeftFingerIKAlpha = Value;
	//UE_LOG(LogTemp, Error, TEXT("LH: Interpol active %f"), Value);
}

void AIAIAvatarCharacter::HandleLeftFingerIKBoneInterpolationProgress(float Value)
{
	FVector New = FMath::Lerp(LeftFingerIKBoneInterpolation.InitialValue, LeftFingerIKBoneInterpolation.TargetValue, Value);

	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);

	AnimationInstance->LeftFingerIKTargetPosition = New;

	//UE_LOG(LogTemp, Error, TEXT("LH: Bone Interpol active %f"), Value);
}

void AIAIAvatarCharacter::StartLeftFingerIKEnablement(FVector NewVec)
{
	if (LeftFingerIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Setting the target of the bone modification
		AnimationInstance->LeftFingerIKTargetPosition = NewVec;

		// Initializing the actual timeline / smooth transition
		LeftFingerIKAlphaInterpolation.InitialValue = AnimationInstance->LeftFingerIKAlpha;
		LeftFingerIKAlphaInterpolation.TargetValue = 1;

		UE_LOG(LogAvatarCharacter, Log, TEXT("LH: Interpolation from %f to %f"), LeftFingerIKAlphaInterpolation.InitialValue,
			LeftFingerIKAlphaInterpolation.TargetValue);

		LeftFingerIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftFingerIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		LeftFingerIKAlphaInterpolation.Timeline.AddInterpFloat(LeftFingerIKCurveFloat, LeftFingerIKAlphaInterpolation.ProgressFunction);
		LeftFingerIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		LeftFingerIKAlphaInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute StartLeftFingerIKEnablement without a LeftFingerIKCurveFloat"));
	}
}

void AIAIAvatarCharacter::InterpolateLeftFingerIKTo(FVector NewVec)
{
	if (LeftFingerIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: InterpolateLeftFingerIKTo (%f,%f,%f)"), NewVec.X, NewVec.Y, NewVec.Z);

		LeftFingerIKBoneInterpolation.InitialValue = AnimationInstance->LeftFingerIKTargetPosition;
		LeftFingerIKBoneInterpolation.TargetValue = NewVec;
		LeftFingerIKBoneInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftFingerIKBoneInterpolationProgress"));

		LeftFingerIKBoneInterpolation.Timeline.AddInterpFloat(LeftFingerIKCurveFloat,
			LeftFingerIKBoneInterpolation.ProgressFunction);
		LeftFingerIKBoneInterpolation.Timeline.SetLooping(false);

		LeftFingerIKBoneInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute InterpolateLeftFingerIKTo without a LeftFingerIKCurveFloat"));
	}
}

void AIAIAvatarCharacter::StartLeftFingerIKDisablement()
{
	if (LeftFingerIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Initializing the actual timeline / smooth transition
		LeftFingerIKAlphaInterpolation.InitialValue = AnimationInstance->LeftFingerIKAlpha;
		LeftFingerIKAlphaInterpolation.TargetValue = 0;

		UE_LOG(LogAvatarCharacter, Log, TEXT("LH: Interpolation from %f to %f"), LeftFingerIKAlphaInterpolation.InitialValue,
			LeftFingerIKAlphaInterpolation.TargetValue);

		LeftFingerIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleLeftFingerIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		LeftFingerIKAlphaInterpolation.Timeline.AddInterpFloat(LeftFingerIKCurveFloat, LeftFingerIKAlphaInterpolation.ProgressFunction);
		LeftFingerIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		LeftFingerIKAlphaInterpolation.Timeline.ReverseFromEnd();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("LH: Can't execute StartLeftFingerIKDisablement without a LeftFingerIKCurveFloat"));
	}
}


// Right Finger IK interpolation Handling
void AIAIAvatarCharacter::HandleRightFingerIKAlphaInterpolationProgress(float Value)
{
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);
	AnimationInstance->RightFingerIKAlpha = Value;
	//UE_LOG(LogTemp, Error, TEXT("RH: Interpol active %f"), Value);
}

void AIAIAvatarCharacter::HandleRightFingerIKBoneInterpolationProgress(float Value)
{
	FVector New = FMath::Lerp(RightFingerIKBoneInterpolation.InitialValue, RightFingerIKBoneInterpolation.TargetValue, Value);

	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
	check(AnimationInstance);

	AnimationInstance->RightFingerIKTargetPosition = New;

	UE_LOG(LogTemp, Error, TEXT("RF: Bone Interpol active %f"), Value);
}

void AIAIAvatarCharacter::StartRightFingerIKEnablement(FVector NewVec)
{
	if (RightFingerIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Setting the target of the bone modification
		AnimationInstance->RightFingerIKTargetPosition = NewVec;

		// Initializing the actual timeline / smooth transition
		RightFingerIKAlphaInterpolation.InitialValue = AnimationInstance->RightFingerIKAlpha;
		RightFingerIKAlphaInterpolation.TargetValue = 1;

		UE_LOG(LogAvatarCharacter, Log, TEXT("RF: Interpolation from to (%f,%f,%f)"), 
			NewVec.X, NewVec.Y, NewVec.Z);

		RightFingerIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightFingerIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		RightFingerIKAlphaInterpolation.Timeline.AddInterpFloat(RightFingerIKCurveFloat, RightFingerIKAlphaInterpolation.ProgressFunction);
		RightFingerIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		RightFingerIKAlphaInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute StartRightFingerIKEnablement without a RightFingerIKCurveFloat"));
	}
}

void AIAIAvatarCharacter::InterpolateRightFingerIKTo(FVector NewVec)
{
	if (RightFingerIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);
		UE_LOG(LogAvatarCharacter, Warning, TEXT("RF: InterpolateRightFingerIKTo (%f,%f,%f)"), NewVec.X, NewVec.Y, NewVec.Z);

		RightFingerIKBoneInterpolation.InitialValue = AnimationInstance->RightFingerIKTargetPosition;
		RightFingerIKBoneInterpolation.TargetValue = NewVec;
		RightFingerIKBoneInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightFingerIKBoneInterpolationProgress"));

		RightFingerIKBoneInterpolation.Timeline.AddInterpFloat(RightFingerIKCurveFloat,
			RightFingerIKBoneInterpolation.ProgressFunction);
		RightFingerIKBoneInterpolation.Timeline.SetLooping(false);

		RightFingerIKBoneInterpolation.Timeline.PlayFromStart();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RF: Can't execute InterpolateRightFingerIKTo without a RightFingerIKCurveFloat"));
	}
}

void AIAIAvatarCharacter::StartRightFingerIKDisablement()
{
	if (RightFingerIKCurveFloat)
	{
		UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());
		check(AnimationInstance);

		// Initializing the actual timeline / smooth transition
		RightFingerIKAlphaInterpolation.InitialValue = AnimationInstance->RightFingerIKAlpha;
		RightFingerIKAlphaInterpolation.TargetValue = 0;

		UE_LOG(LogAvatarCharacter, Log, TEXT("RH: Interpolation from %f to %f"), RightFingerIKAlphaInterpolation.InitialValue,
			RightFingerIKAlphaInterpolation.TargetValue);

		RightFingerIKAlphaInterpolation.ProgressFunction.BindUFunction(this, FName("HandleRightFingerIKAlphaInterpolationProgress"));

		//Setting up the loop status and the function that is going to fire when the timeline ticks
		RightFingerIKAlphaInterpolation.Timeline.AddInterpFloat(RightFingerIKCurveFloat, RightFingerIKAlphaInterpolation.ProgressFunction);
		RightFingerIKAlphaInterpolation.Timeline.SetLooping(false);

		//Starting the timeline...
		RightFingerIKAlphaInterpolation.Timeline.ReverseFromEnd();
	}
	else
	{
		UE_LOG(LogAvatarCharacter, Error, TEXT("RH: Can't execute StartRightFingerIKDisablement without a RightFingerIKCurveFloat"));
	}
}

 void AIAIAvatarCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
	 Super::PostEditChangeProperty(e);

	 //Property : Name of the changed variable. If its inside a struct, you will only see the name of the variable
	 //MemberProperty : Name of the changed variable. If its inside a struct, you will get the name of the struct

	 UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(GetMesh()->GetAnimInstance());

	 if (!AnimationInstance) {
		 UE_LOG(LogTemp, Error, TEXT("AnimationInstance is nullptr in AIAIAvatarCharacter::PostEditChangeProperty. Leaving the AnimationInstance untouched. Exiting ...."));
		 return;
	 }
	 //check(AnimationInstance != nullptr);

	 FName PropertyName = (e.Property != NULL) ? e.MemberProperty->GetFName() : NAME_None;
	 if (PropertyName == GET_MEMBER_NAME_CHECKED(AIAIAvatarCharacter, HandTargetPosition)) {
		UE_LOG(LogAvatarCharacter, Log, TEXT("Changed Left HandTarget"));
		AnimationInstance->LeftHandIKTargetPosition = HandTargetPosition;

		 return;
	 }

	 if (PropertyName == GET_MEMBER_NAME_CHECKED(AIAIAvatarCharacter, RightHandTargetPosition)) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Changed Right HandTarget"));
		 AnimationInstance->RightHandIKTargetPosition = RightHandTargetPosition;

		 return;
	 }

	 if (PropertyName == GET_MEMBER_NAME_CHECKED(AIAIAvatarCharacter, Spine1Rotation)) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Changed Spine1Rotation"));

		 AnimationInstance->Spine1Rotation = Spine1Rotation;

		 return;
	 }
	 
	 if (PropertyName == GET_MEMBER_NAME_CHECKED(AIAIAvatarCharacter, Spine2Rotation)) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Changed Spine2Rotation"));

		 AnimationInstance->Spine2Rotation = Spine2Rotation;

		 return;
	 }

	 if (PropertyName == GET_MEMBER_NAME_CHECKED(AIAIAvatarCharacter, HipRotation)) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Changed HipRotation"));

		 AnimationInstance->HipRotation = HipRotation;

		 return;
	 }

	 if (PropertyName == GET_MEMBER_NAME_CHECKED(AIAIAvatarCharacter, HandRotation)) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Changed Left HandRotation"));

		 AnimationInstance->HandRotation = HandRotation;

		 return;
	 }

	 if (PropertyName == GET_MEMBER_NAME_CHECKED(AIAIAvatarCharacter, RightHandRotation)) {
		 UE_LOG(LogAvatarCharacter, Log, TEXT("Changed Right HandRotation"));

		 AnimationInstance->RightHandRotation = RightHandRotation;

		 return;
	 }
 }

 void AIAIAvatarCharacter::ResetFollowCamera() {
	 FollowCamera->SetRelativeRotation(FRotator(0,0,0));
	 Cast<UIAIAvatarAnimationInstance>(this->GetMesh()->GetAnimInstance())->SkelControl_Head = FRotator(0,0,0);
 }