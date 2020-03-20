// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "TaskAnimParamLogic.h"

// Sets default values for this component's properties
UTaskAnimParamLogic::UTaskAnimParamLogic()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTaskAnimParamLogic::BeginPlay()
{
	Super::BeginPlay();

	// ...

	Avatar = Cast<AIAIAvatarCharacter>(GetOwner());
	check(Avatar != nullptr);

	Animation = Cast<UIAIAvatarAnimationInstance>(Avatar->GetMesh()->GetAnimInstance());
	check(Animation != nullptr);

	bRunAnimation = false;

	currentAnimTime = 0;
}


// Called every frame
void UTaskAnimParamLogic::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (bRunAnimation) {

		SetJointAlphas();
		AnimParams.AnimFunctionDelegate.ExecuteIfBound(currentAnimTime);

		currentAnimTime += DeltaTime;
		if (currentAnimTime > AnimParams.animTime) {
			bRunAnimation = false;
			UnSetJointAlphas();
		}
	}
	else {
		currentAnimTime = 0;
	}
}


AActor* UTaskAnimParamLogic::CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName) {

	FBox ReachArea;
	AActor* Object = NULL;
	FVector ObjLocationInCompSpace = FVector(0, 0, 0);

	// Defining area of proper reach
	ReachArea = FBox::BuildAABB(FVector(45, 0, -10), FVector(25, 20, 5));

	// Filtering objects
	for (auto& It : Objects)
	{
		AActor *Item = It.Value.GetActor();

		if (Item->GetName().Equals(ObjName)) {

			// Get location relative to component space
			ObjLocationInCompSpace = Avatar->GetTransform().InverseTransformPosition(Item->GetActorLocation());

			// Check if within reach area
			if (ReachArea.IsInside(ObjLocationInCompSpace)) {
				Object = Item;
			}
		}
	}

	return Object;
}

void UTaskAnimParamLogic::SetJointAlphas() {

	if (AnimParams.bSet_LH_Loc) 
		Animation->LeftHandIKAlpha = 1;
	if (AnimParams.bSet_RH_Loc)
		Animation->RightHandIKAlpha = 1;
	if (AnimParams.bSet_LH_Rot)
		Animation->HandRotationAlpha = 1;
	if (AnimParams.bSet_RH_Rot)
		Animation->RightHandRotationAlpha = 1;
	if (AnimParams.bSet_LF_Rot)
		Animation->leftHandGraspingAlpha = 1;
	if (AnimParams.bSet_RF_Rot)
		Animation->rightHandGraspingAlpha = 1;
	if (AnimParams.bSet_S01_Rot)
		Animation->SpineRotationAlpha = 1;
}

void UTaskAnimParamLogic::UnSetJointAlphas() {
	if (AnimParams.bSet_LH_Loc)
		Animation->LeftHandIKAlpha = 0;
	if (AnimParams.bSet_RH_Loc)
		Animation->RightHandIKAlpha = 0;
	if (AnimParams.bSet_LH_Rot)
		Animation->HandRotationAlpha = 0;
	if (AnimParams.bSet_RH_Rot)
		Animation->RightHandRotationAlpha = 0;
	if (AnimParams.bSet_LF_Rot)
		Animation->leftHandGraspingAlpha = 0;
	if (AnimParams.bSet_RF_Rot)
		Animation->rightHandGraspingAlpha = 0;
	if (AnimParams.bSet_S01_Rot)
		Animation->SpineRotationAlpha = 0;
}

// Check for cuttable items within a list of unique hit results and filter out those out of proper reach
#pragma optimize("", off)
TArray<AActor*> UTaskAnimParamLogic::CheckForCuttableObjects(TMap<FString, FHitResult> Objects) {

	FBox ReachArea;
	TArray<AActor*> Cuttables;
	FVector ObjLocationInCompSpace = FVector(0,0,0);

	// Defining area of proper reach
	ReachArea = FBox::BuildAABB(FVector(45, 0, -10), FVector(25, 20, 5));
	
	// Filtering non cuttable objects
	for (auto& It : Objects)
	{
		AActor *Item = It.Value.GetActor();

		// Check if cuttable
		if (Item->GetRootComponent()->ComponentHasTag("Cuttable")) {

			// Get location relative to component space
			ObjLocationInCompSpace = Avatar->GetTransform().InverseTransformPosition(Item->GetActorLocation());

			// Check if within reach area
			if (ReachArea.IsInside(ObjLocationInCompSpace)) {
				Cuttables.Add(Item);
			}
		}
	}
	
	return Cuttables;
}
#pragma optimize("", on)

// This will choose the biggest cuttable object 
AActor* UTaskAnimParamLogic::PickOneObject(TArray<AActor*> Cuttables) {
	
	AActor* BiggestActor = nullptr;
	float biggestVolume = 0;
	FBox Box;

	for (auto It = Cuttables.CreateIterator(); It; It++)
	{
		float volume = (*It)->GetComponentsBoundingBox().GetVolume();
		if (volume > biggestVolume) {
			biggestVolume = volume;
			BiggestActor = *It;
		}
	}

	return BiggestActor;
}

// Check if item is in good position for cutting
bool UTaskAnimParamLogic::isInGoodAlignment(CuttableObjectData_t &ItemData) {

	AActor* Item = ItemData.Object;
	FRotator OriginalRot;
	FVector ShortestAxis;
	float shortestAxisLength;
	float smallestAngleToVec = 90;
	float tempAngle;


	// Save Original rotation
	ItemData.OriginalRotation = Item->GetActorRotation();
														// The bounding box is the best dimensional ground data
	// Temporaly align bounding box to mesh				// we can get with no vision.
	Item->SetActorRotation(FRotator(0, 0, 0));          // But it's aligned to world's axes, not Item's axes. 
														// Note: This work under the assumption that the actual
	// Get origin and extension							// mesh is properly aligned to Item's axes. 
	Item->GetActorBounds(false, ItemData.Origin, ItemData.Extent);		

	// Set original rotation
	Item->SetActorRotation(ItemData.OriginalRotation);
	
	// Check height
	if (ItemData.Extent.Z > 40) {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: height %f is way too high for cutting. Minimun is 80."), 2 * ItemData.Extent.Z);
		return false;
	}

	// Find shortest axis in the XY plane
	if (ItemData.Extent.X < ItemData.Extent.Y) {
		ShortestAxis = FVector(1, 0, 0);
		shortestAxisLength = ItemData.Extent.X;
	}
	else {
		ShortestAxis = FVector(0, 1, 0);
		shortestAxisLength = ItemData.Extent.Y;
	}

	// Check width
	if (shortestAxisLength > 10) {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: width %f is too high for cutting. Minimun is 20."), 2 * ItemData.Extent.Z);
		return false;
	}

	// Rotate shortest axis according to actor rotation
	ShortestAxis = ItemData.OriginalRotation.RotateVector(ShortestAxis);

	// Idealy the shortest axis should be aligned to Avatar's forward vector.
	FRotator A = ShortestAxis.ToOrientationRotator();
	FRotator B = Avatar->GetActorForwardVector().ToOrientationRotator();
	tempAngle = A.Yaw - B.Yaw;
	if ((tempAngle > -6 && tempAngle < 6)||
		(tempAngle > 354 && tempAngle < 366)||
		(tempAngle > -366 && tempAngle < -354) ||
		(tempAngle > -186 && tempAngle < -174) ||
		(tempAngle > 174 && tempAngle < 186)
		) {
		ItemData.angle = tempAngle;
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: The angle of %f between Item's short axis and the Avatar is too high for cutting. Good angle is 0° with ±6°. %f"), tempAngle);
		return false;
	}

	return true;
}

// Set parameters for task animation
#pragma optimize("", off)
void UTaskAnimParamLogic::calculateCutAnimParameters(CuttableObjectData_t &ItemData) {

	FTaskAnimParameters_t MyAnimParams;
	bool orderWidthLenght = false;
	FVector HoldPoint;
	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier = FVector(0, 0, 0);
	FVector HoldAdjusment = FVector(0,0,0);
	FVector StartAdjustment = FVector(0, 0, 0);
	FVector EndAdjustment = FVector(0, 0, 0);
	FRotator HoldRotation = FRotator(0, 0, 0);
	FRotator LocalRot;
	float sliceWidth = 2;
	float holdingDistance = 15;
	float halfLength;
	float tmp;

	// Work on longest axis
	if (ItemData.Extent.Y > ItemData.Extent.X) {
		orderWidthLenght = true;
		halfLength = ItemData.Extent.Y;
	}
	else {
		orderWidthLenght = false;
		halfLength = ItemData.Extent.X;
	}


	// Adjust to Item
	if (ItemData.Object->ActorHasTag("Bread")) {

		// Skill
		AnimParams.RH_Loc_Curve = CuttingBreadAnimCurve;
		AnimParams.RH_Rot_Curve = CuttingBreadAnimRotCurve;
		AnimParams.animTime = 10;
		sliceWidth = 2;

		// Right Hand
		HoldAdjusment = FVector(-5, 0, 2);
		EndAdjustment = FVector(-18, 0, 5.3);
		Multiplier = FVector(1, 4, 1);

		// Left Hand
		HoldRotation = FRotator(0, 90, -80);
	}
	else if (ItemData.Object->ActorHasTag("Steak")) {
		
		// Skill
		AnimParams.RH_Loc_Curve = CuttingSteakAnimCurve;
		AnimParams.RH_Rot_Curve = CuttingSteakAnimRotCurve;
		AnimParams.animTime = 2;
		sliceWidth = 1;

		// Right Hand
		HoldAdjusment = FVector(-5, 6.5, 15.2);
		StartAdjustment = FVector(-20, -1, 8.5);
		EndAdjustment = FVector(-20, -1, 8);
		Multiplier = FVector(1, 4, 1);

		// Left Hand
		HoldRotation = FRotator(-30, 160, -87);
	}
	else if (ItemData.Object->ActorHasTag("Zucchini")) {
		
		// Skill
		AnimParams.RH_Loc_Curve = CuttingZucchiniAnimCurve;
		AnimParams.RH_Rot_Curve = CuttingZucchiniAnimRotCurve;
		AnimParams.animTime = 1.6;
		sliceWidth = 1;

		// Right Hand
		HoldAdjusment = FVector(-7, 0, 4);
		StartAdjustment = FVector(-30, -1, 18);
		EndAdjustment = FVector(-30, -1, 8);
		Multiplier = FVector(1, 8, 1);

		// Left Hand
		HoldRotation = FRotator(0, 90, -80);
	}

	// local order is (Width, Lenght, Height)

	// Calculate End Point
	EndPoint.X = 0;
	EndPoint.Y = -1 * halfLength + sliceWidth;
	EndPoint.Z = -1 * ItemData.Extent.Z;
	
	// Calculate hold point
	HoldPoint.X = 0;
	HoldPoint.Y = -1 * halfLength + holdingDistance;
	HoldPoint.Z = ItemData.Extent.Z;

	// Adjust coordinates. The IK used is controling the Avatar's wrist that shouldn't be exactly on
	// the working points but maybe a bit above and behind.
	EndPoint += EndAdjustment;
	HoldPoint += HoldAdjusment;

	// Put back to original order
	if (!orderWidthLenght) {

		tmp = EndPoint.X;
		EndPoint.X = EndPoint.Y;
		EndPoint.Y = tmp;

		tmp = HoldPoint.X;
		HoldPoint.X = HoldPoint.Y;
		HoldPoint.Y = tmp;
	}

	// We are always cutting on the right extreme of the object according to Avatar's point of view.
	if (!(ItemData.angle > 354 || ItemData.angle < -354 || (ItemData.angle < 6 && ItemData.angle > -6))) {
		EndPoint = EndPoint.RotateAngleAxis(180, FVector(0, 0, 1));
		HoldPoint = HoldPoint.RotateAngleAxis(180, FVector(0, 0, 1));
		ItemData.angle -= 180;
	}

	// Calculate Orientation
	LocalRot = FRotator(0, ItemData.angle, 0);

	// Convert coordinates relative to world
	EndPoint   = ItemData.Object->GetActorTransform().TransformPosition(EndPoint);
	HoldPoint  = ItemData.Object->GetActorTransform().TransformPosition(HoldPoint);

	// Convert points relative to Avatar. This is best to adjust points
	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);
	HoldPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(HoldPoint);

	// Start Point is holding position
	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = EndPoint - StartPoint;

	// Put back in world coordinates
	AnimParams.RH_Loc_Curve_Offset = StartPoint;
	AnimParams.RH_Loc_Curve_Multiplier = Multiplier;
	AnimParams.LH_Rotation = HoldRotation;
	AnimParams.LH_Location  = HoldPoint;

	AnimParams.RH_Loc_Curve_Orientation = LocalRot;
	AnimParams.Spine_01_rotation = FRotator(0, 0, 20);

	AnimParams.bSet_LH_Loc = true;
	AnimParams.bSet_LH_Rot = true;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;
}
#pragma optimize("", on)

void UTaskAnimParamLogic::calculateForkAnimParameters(AActor* Target) {

	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier;
	FVector EndAdjustment = FVector(-7, -7, 15);

	AnimParams.Spine_01_rotation = FRotator(0, 0, 15);

	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Target->GetActorLocation());
	EndPoint += EndAdjustment;

	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = (EndPoint - StartPoint);

	//
	AnimParams.RH_Loc_Curve_Offset = StartPoint;
	AnimParams.RH_Loc_Curve_Multiplier = Multiplier;

	// Skill
	AnimParams.RH_Loc_Curve = ForkingAnimCurve;
	AnimParams.RH_Rot_Curve = ForkingAnimRotCurve;

	AnimParams.animTime = 5;

	AnimParams.bSet_LH_Loc = false;
	AnimParams.bSet_LH_Rot = false;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;
}

void UTaskAnimParamLogic::calculateSpoonAnimParameters(AActor* Target) {

	FVector Origin;
	FVector Extent;
	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier;

	FVector EndAdjustment = FVector(-13, -7, 9);
	FVector StartAdjustment = FVector(0, 0, 0);

	AnimParams.Spine_01_rotation = FRotator(0, 0, 20);

	// Get origin and extension							
	Target->GetActorBounds(false, Origin, Extent);

	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Origin);
	EndPoint += EndAdjustment;

	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = (EndPoint - StartPoint);

	//
	AnimParams.RH_Loc_Curve_Offset = StartPoint;
	AnimParams.RH_Loc_Curve_Multiplier = Multiplier;

	// Skill
	AnimParams.RH_Loc_Curve = SpooningSoupAnimCurve;
	AnimParams.RH_Rot_Curve = SpooningSoupAnimRotCurve;

	AnimParams.animTime = 5;

	AnimParams.bSet_LH_Loc = false;
	AnimParams.bSet_LH_Rot = false;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;
}

void UTaskAnimParamLogic::calculatePourAnimParameters(AActor* Target) {

	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier;
	FVector EndAdjustment = FVector(-15, -5, 18);

	AnimParams.Spine_01_rotation = FRotator(0, 0, 15);

	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Target->GetActorLocation());
	EndPoint += EndAdjustment;

	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = (EndPoint - StartPoint);

	//
	AnimParams.RH_Loc_Curve_Offset = StartPoint;
	AnimParams.RH_Loc_Curve_Multiplier = Multiplier;

	// Skill
	AnimParams.RH_Loc_Curve = PouringAnimCurve;
	AnimParams.RH_Rot_Curve = PouringAnimRotCurve;

	AnimParams.animTime = 9;

	AnimParams.bSet_LH_Loc = false;
	AnimParams.bSet_LH_Rot = false;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;
}

// Run cut animation
void UTaskAnimParamLogic::RunCutAnimation(float time) {

	float handYaw;
	FVector Hand_r_Location;
	FRotator Hand_r_Rotation;

	handYaw = AnimParams.RH_Loc_Curve_Orientation.Yaw - 90;
	Hand_r_Rotation = FRotator(70, handYaw, 0);

	Hand_r_Location = AnimParams.RH_Loc_Curve->GetVectorValue(time);
	Hand_r_Location *= AnimParams.RH_Loc_Curve_Multiplier;
	Hand_r_Location += AnimParams.RH_Loc_Curve_Offset;

	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;

	Animation->HandRotation = AnimParams.LH_Rotation;
	Animation->LeftHandIKTargetPosition = AnimParams.LH_Location;
	
	Animation->Spine1Rotation = AnimParams.Spine_01_rotation;
}

// Run fork animation
void UTaskAnimParamLogic::RunForkAnimation(float time) {

	FVector Temp;
	FVector Hand_r_Location;
	FRotator Hand_r_Rotation;
	bool static attached = false;

	if (time == 0) {
		attached = false;
	}

	Temp = AnimParams.RH_Rot_Curve->GetVectorValue(time);
	Hand_r_Rotation = FRotator(Temp.Y,Temp.Z, Temp.X);

	Hand_r_Location = AnimParams.RH_Loc_Curve->GetVectorValue(time);
	Hand_r_Location *= AnimParams.RH_Loc_Curve_Multiplier;
	Hand_r_Location += AnimParams.RH_Loc_Curve_Offset;

	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;
	Animation->Spine1Rotation = AnimParams.Spine_01_rotation;

	if (!attached && time > 2) {
	
		if (Avatar->graspedObject_r->ActorHasTag("DinnerFork")) {
			
			FAttachmentTransformRules attachRules = FAttachmentTransformRules(
				EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld,
				true);

			AnimParams.Object->AttachToActor(Avatar->graspedObject_r, attachRules);
		}
		attached = true;
	}
}

// Run Spoon animation
void UTaskAnimParamLogic::RunSpoonAnimation(float time) {

	FVector Temp;
	FVector Hand_r_Location;
	FRotator Hand_r_Rotation;

	Temp = AnimParams.RH_Rot_Curve->GetVectorValue(time);
	Hand_r_Rotation = FRotator(Temp.Y, Temp.Z, Temp.X);

	Hand_r_Location = AnimParams.RH_Loc_Curve->GetVectorValue(time);
	Hand_r_Location *= AnimParams.RH_Loc_Curve_Multiplier;
	Hand_r_Location += AnimParams.RH_Loc_Curve_Offset;

	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;
	Animation->Spine1Rotation = AnimParams.Spine_01_rotation;
}

// Run fork animation
void UTaskAnimParamLogic::RunPourAnimation(float time) {

	FVector Temp;
	FVector Hand_r_Location;
	FRotator Hand_r_Rotation;

	Temp = AnimParams.RH_Rot_Curve->GetVectorValue(time);
	Hand_r_Rotation = FRotator(Temp.Y, Temp.Z, Temp.X);

	Hand_r_Location = AnimParams.RH_Loc_Curve->GetVectorValue(time);
	Hand_r_Location *= AnimParams.RH_Loc_Curve_Multiplier;
	Hand_r_Location += AnimParams.RH_Loc_Curve_Offset;

	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;
	Animation->Spine1Rotation = AnimParams.Spine_01_rotation;
}


// Process a task request
void UTaskAnimParamLogic::ProcessTask(FString task) {

	AActor* Object;

	if (task.Equals("cut")) {
		Object = PickOneObject(CheckForCuttableObjects(Avatar->ListObjects()));
		if (Object != nullptr) {
			ApplyTaskOnActor(task, Object);
		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: not able to find any cuttable object."));
		}
	}
	else if (task.Equals("Spoon")) {
	}
}

// Process a task request on specific object
void UTaskAnimParamLogic::ApplyTaskOnActor(FString task, AActor* Object) {

	if (Object != NULL) {
		AnimParams.Object = Object;
		if (task.Equals("cut")) {
			CuttableObjectData_t CurrentCutable;
			if (Object->GetRootComponent()->ComponentHasTag("Cuttable")) {
				CurrentCutable.Object = Object;
				if (isInGoodAlignment(CurrentCutable)) {
					calculateCutAnimParameters(CurrentCutable);
					AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunCutAnimation);
					bRunAnimation = true;
				}
			}
		}
		else if (task.Equals("pour")) {
			calculatePourAnimParameters(Object);
			AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunPourAnimation);
			bRunAnimation = true;
		}
		else if (task.Equals("fork")) {
			calculateForkAnimParameters(Object);
			AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunForkAnimation);
			bRunAnimation = true;
		}
		else if (task.Equals("spoon")) {
			calculateSpoonAnimParameters(Object);
			AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunSpoonAnimation);
			bRunAnimation = true;
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

void UTaskAnimParamLogic::ProcessTask_P_ObjectName(FString task, FString ObjectName) {
	
	ApplyTaskOnActor(task, CheckForObject(Avatar->ListObjects(), ObjectName));
}