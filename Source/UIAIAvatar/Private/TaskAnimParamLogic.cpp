// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "TaskAnimParamLogic.h"
#include "Engine.h"

// ***************************************************************************** //
// *************************   DataTableHanlder Class  ************************* //

DataTableHandler::DataTableHandler(FString DTPath) {

	TArray<FString> lines;
	FFileHelper::LoadFileToStringArray(lines, *DTPath);

	for (int i = 0; i < lines.Num(); i++)
	{
		FString aString = lines[i];

		TArray<FString> stringArray = {};

		aString.ParseIntoArray(stringArray, TEXT(","), false);
		TArray<float> row;

		if (stringArray[0].IsNumeric() && stringArray[1].IsNumeric() && stringArray[2].IsNumeric() && stringArray[3].IsNumeric()) {

			row.Add(FCString::Atof(*stringArray[0]));
			row.Add(FCString::Atof(*stringArray[1]));
			row.Add(FCString::Atof(*stringArray[2]));
			row.Add(FCString::Atof(*stringArray[3]));

			ATable.Add(row);
		}
	}

	if (ATable.Num() >= 2) {
		index1 = 0;
		index2 = 1;
	}
	else {
		index1 = 0;
		index2 = index1;
	}
}

float DataTableHandler::GetDuration() {

	if (ATable.Num() != 0) {
		return ATable.Last()[0];
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: Not able to get a Duration. The Data Table is empty."));
		return 0;
	}
}

FVector DataTableHandler::GetVectorValue(float time) {

	FVector Result = FVector(0,0,0);

	float time1 = ATable[index1][0];
	float time2 = ATable[index2][0];

	if (time < time1) {

		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: Time %f s is behind current Data Table's time %f s."), time, time1);

		// Return default
		return Result;
	}

	while (time > time2) {

		index1++;
		index2++;

		if (index2 >= ATable.Num()) {

			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: Time %f s is over max value of %f s."), time, time2);

			// Return default
			return Result;
		}

		time1 = ATable[index1][0];
		time2 = ATable[index2][0];
	}

	// ***** Interpolation *****
	// Get values from table
	FVector Vector1 = FVector(ATable[index1][1], ATable[index1][2], ATable[index1][3]);
	FVector Vector2 = FVector(ATable[index2][1], ATable[index2][2], ATable[index2][3]);

	// Get deltas
	FVector DeltaVector = Vector2 - Vector1;
	FVector OffsetVector = FVector(0,0,0);
	float deltaTime = time2 - time1;
	float offsetTime = time - time1;
	
	// Interpolate
	if (deltaTime > 0) {
		OffsetVector = DeltaVector * offsetTime / deltaTime;
	}
	
	// Get value
	Result = Vector1 + OffsetVector;

	return Result;
}

// ***************************************************************************** //
// ************************* UTaskAnimParamLogic Class ************************* //

// Sets default values for this component's properties
UTaskAnimParamLogic::UTaskAnimParamLogic()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<UDataTable>
		RH_DataTable(TEXT("DataTable'/UIAIAvatar/Animation_Assets/AnimDataTables/CuttingBread/RH_Curve.RH_Curve'"));
	RH_Table = RH_DataTable.Object;
	RH_Table->EmptyTable();

	ConstructorHelpers::FObjectFinder<UDataTable>
		RH_Rot_DataTable(TEXT("DataTable'/UIAIAvatar/Animation_Assets/AnimDataTables/CuttingBread/RH_Rot_Curve.RH_Rot_Curve'"));
	RH_Rot_Table = RH_Rot_DataTable.Object;
	RH_Rot_Table->EmptyTable();

	ConstructorHelpers::FObjectFinder<UDataTable>
		LH_DataTable(TEXT("DataTable'/UIAIAvatar/Animation_Assets/AnimDataTables/CuttingBread/LH_Curve.LH_Curve'"));
	LH_Table = LH_DataTable.Object;
	LH_Table->EmptyTable();

	ConstructorHelpers::FObjectFinder<UDataTable>
		LH_Rot_DataTable(TEXT("DataTable'/UIAIAvatar/Animation_Assets/AnimDataTables/CuttingBread/LH_Rot_Curve.LH_Rot_Curve'"));
	LH_Rot_Table = LH_Rot_DataTable.Object;
	LH_Rot_Table->EmptyTable();

	ConstructorHelpers::FObjectFinder<UDataTable>
		Spine1_DataTable(TEXT("DataTable'/UIAIAvatar/Animation_Assets/AnimDataTables/CuttingBread/Spine1_Rot_Curve.Spine1_Rot_Curve'"));
	S1_Rot_Table = Spine1_DataTable.Object;
	S1_Rot_Table->EmptyTable();

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
	speedFactor = 1;

	writeTime = 0;
	recorded = false;
}

// Called every frame
void UTaskAnimParamLogic::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	float static delay = 0;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (bRunAnimation) {

		if(currentAnimTime == 0) 
			SetJointAlphas();

		AnimParams.AnimFunctionDelegate.ExecuteIfBound(currentAnimTime);

		currentAnimTime += DeltaTime * speedFactor;
		if (currentAnimTime >= AnimParams.animTime) {
			bRunAnimation = false;
			currentAnimTime = 0;
			UnSetJointAlphas();

			if (pendingStates > 0) {
				AnimChain.ExecuteIfBound(pendingStates);
			}
		}
	}
	else {
		currentAnimTime = 0;
	}

	/*
	if (delay > 10) {
		WriteCSV(writeTime);
		writeTime += DeltaTime;
	}

	delay += DeltaTime;
	*/
}

// ****************** Slicing Help Functions ******************** 

// Check for cuttable items within a list of unique hit results and filter out those out of proper reach
#pragma optimize("", off)
TArray<AActor*> UTaskAnimParamLogic::CheckForCuttableObjects(TMap<FString, FHitResult> Objects) {

	FBox ReachArea;
	TArray<AActor*> Cuttables;
	FVector ObjLocationInCompSpace = FVector(0, 0, 0);

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
bool UTaskAnimParamLogic::isInGoodAlignment(ObjectData_t &ItemData) {

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
	if ((tempAngle > -6 && tempAngle < 6) ||
		(tempAngle > 354 && tempAngle < 366) ||
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

// ****************** Help Functions ******************** 

void UTaskAnimParamLogic::AttachObject() {

	// Attachment variables
	FVector  NewObjLocation = FVector(0, 0, 0);
	FRotator NewObjRotation = FRotator(0, 0, 0);
	UStaticMeshComponent *ObjectMesh;
	FFingerRots_t NewFingerRots;
	FName    socket;
	bool keepWorldRot = false;
	bool keepWorldLoc = false;

	FAttachmentTransformRules attachRules = FAttachmentTransformRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		true);

	// Default Fingers' Rotations
	NewFingerRots = GetCurrentFingersRots(AnimParams.bUsingRightHand);

	// Get Animation
	UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(Avatar->GetMesh()->GetAnimInstance());
	check(AnimationInstance != nullptr);

	// Get Static Meshes
	auto AvatarMesh = Avatar->GetMesh();

	ObjectMesh = Cast<UStaticMeshComponent>(AnimParams.Object->GetComponentByClass(UStaticMeshComponent::StaticClass()));

	// Object specific settings
	if (AnimParams.Object->ActorHasTag(TEXT("MilkBox"))) {				// ** Milk Box **

		// Attaching Rule
		attachRules.RotationRule = EAttachmentRule::KeepRelative;

		// Object Location
		NewObjLocation = FVector(-3, 5.5, 0);

		// Object Rotation
		if (AnimParams.bUsingRightHand) {
			NewObjRotation = FRotator(-10, 180, 0);
		}
		else {
			NewObjRotation = FRotator(10, 0, -180);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("BreadKnife"))) {		// ********** Bread Knife *********

		// Object Location
		NewObjLocation = FVector(-2, 2, 9);

		// Object Rotation
		if (AnimParams.bUsingRightHand) {
			NewObjRotation = FRotator(70, -180, -90);
		}
		else {
			NewObjRotation = FRotator(288, 0, -255);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("CookKnife"))) {		// ********** Cook Knife *********

		// Object Location
		NewObjLocation = FVector(-4, 2, 10);

		if (AnimParams.bUsingRightHand) {
			// Object Rotation
			NewObjRotation = FRotator(110.5, 0, 90);
		}
		else {
			NewObjRotation = FRotator(-69.5, 0, 90);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("SpoonSoup"))) {		// ********* Spoon Soup *********

	   // Collision
	   //#if WITH_FLEX
	   //ObjectMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Flex, ECollisionResponse::ECR_Block);
	   //#endif
		// Object Location		 // ud  lr  bf
		NewObjLocation = FVector(-1.9, 7.6, 0.3);

		// Object Rotation
		if (AnimParams.bUsingRightHand) {      // p   y  r
			NewObjRotation = FRotator(-36, 101, -24);
		}
		else {
			NewObjRotation = FRotator(28, -120, 180);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("DinnerFork"))) {		// ********* Dinner Fork *********

		// Object Rotation
		if (AnimParams.bUsingRightHand) {      // p   y  r

			// Object Location		 // ud  lr  bf
			NewObjLocation = FVector(-1.9, 7.6, 0.3);
			NewObjRotation = FRotator(-36, 101, -24);
		}
		else {

			// Object Location		 // ud  lr  bf
			NewObjLocation = FVector(-7.85, 3, 1.55);
			NewObjRotation = FRotator(-5, -5, -89);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("Slice"))
		&& AnimParams.Object->ActorHasTag(TEXT("Bread"))) {		// ********* Bread Slice *********

		UE_LOG(LogAvatarCharacter, Log, TEXT("Bread Slice Grasping"));

		// Object Location		 // ud  lr  bf
		NewObjLocation = FVector(-1, 4, 0);

		// Object Rotation
		if (AnimParams.bUsingRightHand) {      // p   y  r
			NewObjRotation = FRotator(90, 0, 0);
		}
		else {
			NewObjRotation = FRotator(90, 0, 0);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("Bowl"))) {			// ********* Bowl *********

		// Attaching Rule
		attachRules.RotationRule = EAttachmentRule::KeepRelative;

		// Object Location
		NewObjLocation = FVector(-3.5, 9.9, -0.5); // original

		// Object Rotation
		if (AnimParams.bUsingRightHand) {
			NewObjRotation = FRotator(-10, 0, 0);
		}
		else {
			NewObjRotation = FRotator(170, 0, 0);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("Jug"))) {		// ********** Milk Jug *********

		if (AnimParams.bUsingRightHand) {
			// Object Rotation
			NewObjRotation = FRotator(0, -30, 0);
		}
		else {
			NewObjRotation = FRotator(-69.5, 0, 90);
		}
	}
	else if (AnimParams.Object->ActorHasTag(TEXT("Plate"))) {		// ********** Plate *********
		// Attaching Rule
		attachRules.RotationRule = EAttachmentRule::KeepWorld;
		attachRules.LocationRule = EAttachmentRule::KeepWorld;
		keepWorldLoc = true;
		keepWorldRot = true;
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
	if (AnimParams.bUsingRightHand) {
		socket = TEXT("hand_rSocket");
		Avatar->isGrasped_r = true;
		Avatar->graspedObject_r = AnimParams.Object;
	}
	else {
		socket = TEXT("hand_lSocket");
		Avatar->isGrasped_l = true;
		Avatar->graspedObject_l = AnimParams.Object;
		NewObjLocation *= -1;
	}

	// Attach Object to hand socket
	AnimParams.Object->AttachToComponent(AvatarMesh, attachRules, socket);

	//ObjectMesh->BodyInstance.SetResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	if (!keepWorldLoc)
		ObjectMesh->SetRelativeLocation(NewObjLocation);
	if (!keepWorldRot)
		ObjectMesh->SetRelativeRotation(NewObjRotation);

	ObjectMesh->SetSimulatePhysics(false);
}

AActor* UTaskAnimParamLogic::CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName) {

	FBox ReachArea;
	AActor* Object = NULL;
	FVector ObjLocationInCompSpace = FVector(0, 0, 0);

	// Defining area of proper reach
	ReachArea = FBox::BuildAABB(FVector(45, 0, -20), FVector(25, 35, 15));

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
	if (AnimParams.bSet_LI_Loc)
		Animation->LeftFingerIKAlpha = 1;
	if (AnimParams.bSet_LF_Rot)
		Animation->leftHandGraspingAlpha = 1;
	if (AnimParams.bSet_RI_Loc)
		Animation->RightFingerIKAlpha = 1;
	if (AnimParams.bSet_RF_Rot)
		Animation->rightHandGraspingAlpha = 1;
	if (AnimParams.bSet_S01_Rot)
		Animation->SpineRotationAlpha = 1;
	if (AnimParams.bSet_Head_Rot)
		Animation->HeadRotationAlpha = 1;
	if (AnimParams.bSet_Jaw_Rot)
		Animation->JawRotationAlpha = 1;
}

void UTaskAnimParamLogic::UnSetJointAlphas() {
	if (AnimParams.bUnSet_LH_Loc)
		Animation->LeftHandIKAlpha = 0;
	if (AnimParams.bUnSet_RH_Loc)
		Animation->RightHandIKAlpha = 0;
	if (AnimParams.bUnSet_LH_Rot)
		Animation->HandRotationAlpha = 0;
	if (AnimParams.bUnSet_RH_Rot)
		Animation->RightHandRotationAlpha = 0;
	if (AnimParams.bUnSet_LI_Loc)
		Animation->LeftFingerIKAlpha = 0;
	if (AnimParams.bUnSet_LF_Rot)
		Animation->leftHandGraspingAlpha = 0;
	if (AnimParams.bUnSet_RI_Loc)
		Animation->RightFingerIKAlpha = 0;
	if (AnimParams.bUnSet_RF_Rot)
		Animation->rightHandGraspingAlpha = 0;
	if (AnimParams.bUnSet_S01_Rot)
		Animation->SpineRotationAlpha = 0;
	if (AnimParams.bUnSet_Head_Rot)
		Animation->HeadRotationAlpha = 0;
	if (AnimParams.bUnSet_Jaw_Rot)
		Animation->JawRotationAlpha = 0;
}

void UTaskAnimParamLogic::SaveOriginalPose() {
	OriginalPose = GetCurrentAvatarPose();
}

FFingerRots_t UTaskAnimParamLogic::GetCurrentFingersRots(bool isRightHand) {

	FFingerRots_t FingersRots;
	FRotator TempRotIn;
	FRotator TempRotOut;
	FVector TempVecIn;
	FVector TempVecOut;

	if (isRightHand) {
		
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("index_01_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.index_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("index_02_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("index_01_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.index_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("index_03_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("index_02_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.index_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("middle_01_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.middle_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("middle_02_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("middle_01_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.middle_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("middle_03_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("middle_02_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.middle_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("ring_01_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.ring_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("ring_02_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("ring_01_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.ring_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("ring_03_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("ring_02_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.ring_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("pinky_01_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.pinky_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("pinky_02_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("pinky_01_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.pinky_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("pinky_03_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("pinky_02_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.pinky_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("thumb_01_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.thumb_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("thumb_02_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("thumb_01_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.thumb_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("thumb_03_r", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("thumb_02_r", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.thumb_03 = TempRotOut;
	}
	else {
		
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("index_01_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.index_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("index_02_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("index_01_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.index_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("index_03_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("index_02_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.index_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("middle_01_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.middle_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("middle_02_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("middle_01_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.middle_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("middle_03_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("middle_02_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.middle_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("ring_01_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.ring_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("ring_02_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("ring_01_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.ring_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("ring_03_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("ring_02_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.ring_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("pinky_01_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.pinky_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("pinky_02_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("pinky_01_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.pinky_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("pinky_03_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("pinky_02_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.pinky_03 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("thumb_01_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("hand_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.thumb_01 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("thumb_02_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("thumb_01_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.thumb_02 = TempRotOut;
		TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("thumb_03_l", EBoneSpaces::WorldSpace).Rotator();
		Avatar->GetMesh()->TransformToBoneSpace("thumb_02_l", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
		FingersRots.thumb_03 = TempRotOut;
	}

	return FingersRots;
}

FAvatarPose_t UTaskAnimParamLogic::GetCurrentAvatarPose() {
	FAvatarPose_t Pose;

	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	Pose.Spine_01_Rot = TempRotOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	Pose.Head_Rot = TempRotOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("jaw", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("head", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	Pose.Jaw_Rot = TempRotOut;

	Pose.RH_IndexLoc = Avatar->GetMesh()->GetBoneLocation("index_end_r", EBoneSpaces::WorldSpace);
	Pose.LH_IndexLoc = Avatar->GetMesh()->GetBoneLocation("index_end_l", EBoneSpaces::WorldSpace);

	Pose.RH_FingerRots = GetCurrentFingersRots(true);
	Pose.LH_FingerRots = GetCurrentFingersRots(false);

	Pose.RH_Loc = Avatar->GetMesh()->GetBoneLocation("hand_r", EBoneSpaces::ComponentSpace);
	Pose.RH_Rot = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();

	Pose.LH_Loc = Avatar->GetMesh()->GetBoneLocation("hand_l", EBoneSpaces::ComponentSpace);
	Pose.LH_Rot = Avatar->GetMesh()->GetBoneQuaternion("hand_l", EBoneSpaces::ComponentSpace).Rotator();

	return Pose;
}

FRotator UTaskAnimParamLogic::CalculateSpineRot(FVector LocalEndPoint, float elongation) {

	FRotator SpineRotEndPoint = FRotator(0, 0, 0);
	FVector tmp1, Direction;
	FVector ShoulderVector;
	FVector ShoulderVectorO = FVector(0, 0, 43);
	FVector SpineHeight = FVector(0, 0, 110);
	FVector ShouldersLocation;
	float distance, spineAngle = 0;
	do {
		SpineRotEndPoint.Yaw = spineAngle;
		ShoulderVector = ShoulderVectorO.RotateAngleAxis(spineAngle, FVector(-1, 0, 0));
		ShouldersLocation = ShoulderVector + SpineHeight;
		Direction = LocalEndPoint - ShouldersLocation;
		Direction.ToDirectionAndLength(tmp1, distance);
		spineAngle += 5;
	} while (distance > elongation && spineAngle <= 90);

	return SpineRotEndPoint;
}

FRotator UTaskAnimParamLogic::CalculateHeadRot(FVector Point) {

	UE_LOG(LogAvatarCharacter, Log, TEXT("Called 'look to %f %f %f' ."), Point.X, Point.Y, Point.Z);

	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;
	FVector HeadLoc = Avatar->GetMesh()->GetBoneLocation(TEXT("neck_01"), EBoneSpaces::WorldSpace);
	FVector DirVec = HeadLoc - Point;
	TempRotIn = DirVec.ToOrientationRotator();

	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);

	float temp = TempRotOut.Roll;
	TempRotOut.Roll = TempRotOut.Pitch;
	TempRotOut.Yaw += 90;
	TempRotOut.Pitch = 0; // = temp - 90; // Or = 0

	// Limit rotation so Avatar doesn't look wierd
	if (TempRotOut.Yaw > 35) {
		TempRotOut.Yaw = 35;
	}

	UE_LOG(LogAvatarCharacter, Log, TEXT("Rotator: %f %f %f' ."), TempRotOut.Roll, TempRotOut.Pitch, TempRotOut.Yaw);

	return TempRotOut;
}

FVector UTaskAnimParamLogic::CalculateReachBookLocation(AActor *Book, FName Tag, bool bWorldRelative) {

	TArray<UActorComponent*> boxes = Book->GetComponentsByTag(UBoxComponent::StaticClass(), Tag);
	USceneComponent *box = Cast<USceneComponent>(boxes[0]);
	FVector EndPoint;

	if (bWorldRelative) {
		// Relative to World
		EndPoint = box->GetComponentLocation();
	}
	else {
		// Relative to Object
		EndPoint = box->RelativeLocation;
	}
	return EndPoint;
}

// **************** EndPose Calculators *****************

void UTaskAnimParamLogic::Calculate_PointBook_EndPose_Curves(float vPageLoc, float hPageLoc, FAvatarPose_t &EndPose, FCurvesSet_t &Curves) {
	
	FVector EndPoint;
	FVector Origin;
	FVector Extent;
	FVector Scale;

	Scale = AnimParams.Object->GetActorScale();
	AnimParams.Object->GetActorBounds(true, Origin, Extent);
	EndPoint.Z = 1;
	EndPoint.X = Extent.Y/Scale.Y * vPageLoc;
	EndPoint.Y = -Extent.X/Scale.X * hPageLoc;

	UE_LOG(LogAvatarCharacter, Log, TEXT("Extent %s - %f %f"), *Extent.ToString(), vPageLoc, hPageLoc);
	UE_LOG(LogAvatarCharacter, Log, TEXT("Scale %s"), *Scale.ToString());

	// Relative to world
	EndPoint = AnimParams.Object->GetActorTransform().TransformPosition(EndPoint);

	if (AnimParams.Object->ActorHasTag("book")) {
		if (AnimParams.bUsingRightHand) {

			EndPose.RH_Rot = FRotator(50, -90, 90);
			EndPose.RH_FingerRots.thumb_01 = FRotator(-35, -20, 70);
			EndPose.RH_FingerRots.thumb_02 = FRotator(0, -60, 0);
			EndPose.RH_FingerRots.thumb_03 = FRotator(0, -23, 0);

			EndPose.RH_FingerRots.index_01 = FRotator(0, -50, -12);
			EndPose.RH_FingerRots.index_02 = FRotator(-0, -80, 0);
			EndPose.RH_FingerRots.index_03 = FRotator(-0, -50, 0);

			EndPose.RH_FingerRots.middle_01 = FRotator(0, -60, -10);
			EndPose.RH_FingerRots.middle_02 = FRotator(-0, -70, 0);
			EndPose.RH_FingerRots.middle_03 = FRotator(-0, -60, 0);

			EndPose.RH_FingerRots.ring_01 = FRotator(-0, -70, -10);
			EndPose.RH_FingerRots.ring_02 = FRotator(-0, -70, -0);
			EndPose.RH_FingerRots.ring_03 = FRotator(-0, -60, -0);

			EndPose.RH_FingerRots.pinky_01 = FRotator(-0, -90, -25);
			EndPose.RH_FingerRots.pinky_02 = FRotator(-0, -60, -0);
			EndPose.RH_FingerRots.pinky_03 = FRotator(-0, -60, -0);
			
		} 
		else if (AnimParams.bUsingLeftHand) {

			EndPose.LH_Rot = FRotator(-50, 90, -90);
			EndPose.LH_FingerRots.thumb_01 = FRotator(-35, -20, 70);
			EndPose.LH_FingerRots.thumb_02 = FRotator(0, -60, 0);
			EndPose.LH_FingerRots.thumb_03 = FRotator(0, -23, 0);

			EndPose.LH_FingerRots.index_01 = FRotator(0, -50, -12);
			EndPose.LH_FingerRots.index_02 = FRotator(-0, -80, 0);
			EndPose.LH_FingerRots.index_03 = FRotator(-0, -50, 0);

			EndPose.LH_FingerRots.middle_01 = FRotator(0, -60, -10);
			EndPose.LH_FingerRots.middle_02 = FRotator(-0, -70, 0);
			EndPose.LH_FingerRots.middle_03 = FRotator(-0, -60, 0);

			EndPose.LH_FingerRots.ring_01 = FRotator(-0, -70, -10);
			EndPose.LH_FingerRots.ring_02 = FRotator(-0, -70, -0);
			EndPose.LH_FingerRots.ring_03 = FRotator(-0, -60, -0);

			EndPose.LH_FingerRots.pinky_01 = FRotator(-0, -90, -25);
			EndPose.LH_FingerRots.pinky_02 = FRotator(-0, -60, -0);
			EndPose.LH_FingerRots.pinky_03 = FRotator(-0, -60, -0);
		}
	}

	// Head
	EndPose.Head_Rot = CalculateHeadRot(EndPoint);

	// Relative to avatar
	FVector Local = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);

	// Spine
	EndPose.Spine_01_Rot = CalculateSpineRot(Local, 80);

	if (AnimParams.bUsingRightHand) {
		EndPose.RH_IndexLoc = EndPoint;
	}
	else if (AnimParams.bUsingLeftHand) {
		EndPose.LH_IndexLoc = EndPoint;
	}
}

void UTaskAnimParamLogic::Calculate_PassPage_EndPose_Curves(FAvatarPose_t &EndPose, FCurvesSet_t &Curves) {

	// Pass Page Box Location 
	FVector EndPoint = CalculateReachBookLocation(AnimParams.Object, *AnimParams.ActionContext, false);

	// Get oposite side of book
	EndPoint.Y *= -1;

	// Get looking point
	FVector LookingPoint = EndPoint;
	LookingPoint = AnimParams.Object->GetActorTransform().TransformPosition(LookingPoint);
	EndPose.Head_Rot = CalculateHeadRot(LookingPoint);

	// Modify end point for multiplier 
	EndPoint.Y += 10;
	EndPoint.Z += EndPoint.Y;

	// Relative to world
	EndPoint = AnimParams.Object->GetActorTransform().TransformPosition(EndPoint);

	// Relative to Avatar
	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);

	EndPoint = EndPoint + FVector(0, 0, 5);

	if (AnimParams.bUsingRightHand) {
		EndPose.RH_Loc = EndPoint;
	}
	else {
		EndPose.LH_Loc = EndPoint;
	}

	// Hand Rotation trajectory
	// Pitch Yaw  Roll
	//  Y     Z     X
	// 60      0   120 
	//  0    -90    90
	// 60   -150   120 
	EndPose.RH_Rot = FRotator(0, -150, 90);

	Curves = PassingPageCurves;

}

// **************** Setting Up Animation Chains *****************

// Start passing page animation chain
void UTaskAnimParamLogic::StartPassPageAnimChain(AActor *Target, bool bLast, FString Hand) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	FVector RH_LocEndPoint_Adjustment;
	FVector LH_LocEndPoint_Adjustment;

	speedFactor = 1;
	pendingStates = 2;
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunPassPageAnimChain);
	SaveOriginalPose();

	AnimParams.ClearJointFlags();
	AnimParams.bUsingRightHand = false;
	AnimParams.bUsingLeftHand = false;

	AnimParams.Object = Target;

	if (bLast) {
		AnimParams.ActionContext = "ClosingPageBox";
	}
	else {
		AnimParams.ActionContext = "TurnPageBox";
	}

	if (Hand.Equals("left") && !Avatar->isGrasped_l) {
		AnimParams.bUsingLeftHand = true;
	}
	else if (Hand.Equals("right") && !Avatar->isGrasped_r) {
		AnimParams.bUsingRightHand = true;
	}
	else {
		UE_LOG(LogAvatarCharacter, Log, TEXT("The intended hand is already being used!!"));
		return;
	}

	// Set params for first animation in chain ************************
	// Grasping page

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	// Calculate EndPose
	FVector EndPoint = CalculateReachBookLocation(Target, *AnimParams.ActionContext, true);

	if (Target->ActorHasTag("Book")) {
		if (AnimParams.bUsingRightHand) {

			RH_LocEndPoint_Adjustment = FVector(-3, 0, 10);
			EndPose.RH_Rot = FRotator(60, 0, 120);

			EndPose.RH_FingerRots.thumb_01 = FRotator(10, -50, 140);
			EndPose.RH_FingerRots.thumb_02 = FRotator(0, -0, 0);
			EndPose.RH_FingerRots.thumb_03 = FRotator(0, -0, 0);

			EndPose.RH_FingerRots.index_01 = FRotator(-10, -65, 0);
			EndPose.RH_FingerRots.index_02 = FRotator(-0, -45, 0);
			EndPose.RH_FingerRots.index_03 = FRotator(-0, -45, 0);

			EndPose.RH_FingerRots.middle_01 = FRotator(-15, -65, 0);
			EndPose.RH_FingerRots.middle_02 = FRotator(-0, -45, 0);
			EndPose.RH_FingerRots.middle_03 = FRotator(-0, -45, 0);

			EndPose.RH_FingerRots.ring_01 = FRotator(-10, -45, 0);
			EndPose.RH_FingerRots.pinky_01 = FRotator(-10, -45, 0);
		}
	}

	EndPose.Head_Rot = CalculateHeadRot(EndPoint);

	if (AnimParams.bUsingRightHand) {
		EndPose.RH_Loc = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);
		EndPose.RH_Loc += RH_LocEndPoint_Adjustment;
		EndPose.Spine_01_Rot = CalculateSpineRot(EndPose.RH_Loc, 65);
	}

	if (AnimParams.bUsingLeftHand) {
		EndPose.LH_Loc = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);
		EndPose.LH_Loc += LH_LocEndPoint_Adjustment;
		EndPose.Spine_01_Rot = CalculateSpineRot(EndPose.LH_Loc, 65);
	}

	// Set corresponding curves
	Curves = ReachingCurves;

	SetAnimParams(StartPose, EndPose, Curves);
}

// Start point book animation chain
void UTaskAnimParamLogic::StartReadNewspaperAnimChain(AActor *Target, FString Hand) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	AnimParams.bUsingRightHand = false;
	AnimParams.bUsingLeftHand = false;

	pendingStates = 17;
	SaveOriginalPose();
	AnimParams.Object = Target;
	AnimParams.ActionContext = "TurnPageBox"; // This is a hack to get a location where to point
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunReadNewspaperAnimChain);
	speedFactor = 1;

	if (Hand.Equals("left") && !Avatar->isGrasped_l) {
		AnimParams.bUsingLeftHand = true;
	}
	else if (Hand.Equals("right") && !Avatar->isGrasped_r) {
		AnimParams.bUsingRightHand = true;
	}
	else {
		UE_LOG(LogAvatarCharacter, Log, TEXT("The intended hand is already being used!!"));
		return;
	}

	// Set params for first animation in chain ************************

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;
	float page_start = 0;
	if (AnimParams.bUsingLeftHand) page_start = -1;
	Calculate_PointBook_EndPose_Curves(-0.35, page_start + 0.15, EndPose, Curves);

	// Set corresponding curves
	Curves = ReachingCurves;

	SetAnimParams(StartPose, EndPose, Curves);
}

// Start grasping animation chain
void UTaskAnimParamLogic::StartGraspingAnimChain(AActor *Target, FString Hand, bool bHold) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	pendingStates = 2;
	SaveOriginalPose();
	speedFactor = 2;
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunGraspingAnimChain);
	
	AnimParams.ClearJointFlags();
	AnimParams.bUsingRightHand = false;
	AnimParams.bUsingLeftHand = false;

	AnimParams.Object = Target;

	// Getting location relative to component
	FVector ObjLocationInCompSpace = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Target->GetActorLocation());

	if (Hand.Equals("left") && !Avatar->isGrasped_l) {
		AnimParams.bUsingLeftHand = true;
	}
	else if (Hand.Equals("right") && !Avatar->isGrasped_r) {
		AnimParams.bUsingRightHand = true;
	}
	else if (Hand.Equals("any")) {

		// Verify my hands are not busy
		if (Avatar->isGrasped_l && Avatar->isGrasped_r) {
			UE_LOG(LogAvatarCharacter, Log, TEXT("Both my hands are busy! I'm not an octopus!!"));
			return;
		}
		else {
			
			if (Avatar->isGrasped_r) {	// Use left hand if right hand is busy,
										// no matter where the object is.
				AnimParams.bUsingLeftHand = true;
			}
			else if (ObjLocationInCompSpace.X > 0 && !Avatar->isGrasped_l) {
				// If right hand is free, then only use left hand 
				// as loong as it is also free and object is on left side.
				AnimParams.bUsingLeftHand = true;
			}
			else {
				AnimParams.bUsingRightHand = true;
			}
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Log, TEXT("The intended hand is already being used!!"));
		return;
	}

	// Set params for first animation in chain ************************
	// Prepare for grasping

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	// Calculate EndPose

	// Pre step for reaching object
	if (AnimParams.bUsingRightHand) {
		EndPose.RH_Loc = FVector(-20, 15, 100);
		EndPose.RH_Rot = FRotator(45, -110, 60);
	}

	if (AnimParams.bUsingLeftHand) {
		EndPose.LH_Loc = FVector(20, 15, 100);
		EndPose.LH_Rot = FRotator(45, -110, 60);
	}

	// Set corresponding curves
	Curves = ReachingCurves;

	SetAnimParams(StartPose, EndPose, Curves);
}

// Start placing animation chain
void UTaskAnimParamLogic::StartPlacingAnimChain(FString targetPlace, FString Hand, FVector Point)
{
	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	// Local variables
	TMap<FString, FHitResult> MyUniqueHits;
	FVector TargetLocation;

	pendingStates = 1;
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunPlacingAnimChain);
	speedFactor = 0.7;

	AnimParams.ClearJointFlags();
	AnimParams.bUsingLeftHand = false;
	AnimParams.bUsingRightHand = false;

	// Check hands and if they have any object
	if (Hand.Equals("right")) {
		if (!Avatar->isGrasped_r) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: There is nothing to place from right hand."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("ERROR: There is nothing to place from right hand."), true, FVector2D(1.5, 1.5));
			return;
		}
		AnimParams.bUsingRightHand = true;
	}
	else if (Hand.Equals("left")) {
		if (!Avatar->isGrasped_l) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: There is nothing to place from left hand."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("ERROR: There is nothing to place from left hand."), true, FVector2D(1.5, 1.5));
			return;
		}
		AnimParams.bUsingLeftHand = true;
	}
	else if (Hand.Equals("any")) {
		if (Avatar->isGrasped_r) {
			AnimParams.bUsingRightHand = true;
		}
		else if (!Avatar->isGrasped_l) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: There is nothing to place from any hand."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, TEXT("ERROR: There is nothing to place from any hand."), true, FVector2D(1.5, 1.5));
			return;
		}
		else {
			AnimParams.bUsingLeftHand = true;
		}
	}

	// Set params for first animation in chain ************************
	// Reach placing location

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	// Calculate EndPose
	if (targetPlace.Equals("VECTOR")) {

		FVector socketAbsLoc;
		FVector loc;
		FVector offset;

		if (AnimParams.bUsingRightHand) {
			socketAbsLoc = Avatar->GetMesh()->GetBoneLocation(TEXT("hand_r"));
			loc = Avatar->graspedObject_r->GetActorLocation();
			offset = Avatar->graspedObject_r->GetActorLocation() - socketAbsLoc;
		}
		else {
			socketAbsLoc = Avatar->GetMesh()->GetBoneLocation(TEXT("hand_l"));
			loc = Avatar->graspedObject_l->GetActorLocation();
			offset = Avatar->graspedObject_l->GetActorLocation() - socketAbsLoc;
		}

		EndPose.Head_Rot = CalculateHeadRot(Point);

		Point -= offset;

		TargetLocation = Point;
		TargetLocation = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(TargetLocation);
	}
	else if (targetPlace.Equals("microwave")) {
		MyUniqueHits = Avatar->ListObjects();

		// Verify microwave is near
		if (MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor() == NULL) {
			UE_LOG(LogAvatarCharacter, Error, TEXT("ERROR: Microwave is not near you."));
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: The Microwave is not near you.")), true, FVector2D(1.7, 1.7));
			return;
		}
		else {
			// Getting location relative to component
			TargetLocation = MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor()->GetActorLocation();
			FRotator TargetRotation = MyUniqueHits.FindRef("MachineStep05_Edelstahl_2").GetActor()->GetActorRotation();
			TargetLocation += TargetRotation.RotateVector(FVector(0, -5, -10));

			EndPose.Head_Rot = CalculateHeadRot(TargetLocation);
			TargetLocation = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(TargetLocation);
		}
	}
	else if (targetPlace.Equals("table")) {
		// Define location.
		if (AnimParams.bUsingRightHand) {
			TargetLocation = FVector(-30, 42, 90);
		}
		else {
			TargetLocation = FVector(30, 42, 90);
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Warning, TEXT("Warning: Unrecognized place \"%s\". Just placing here."), *targetPlace);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Yellow, FString::Printf(TEXT("Warning: Unrecognized place \"%s\". Just placing here."), *targetPlace), true, FVector2D(1.5, 1.5));
		return;
	}

	EndPose.Spine_01_Rot = CalculateSpineRot(TargetLocation, 65);

	if (AnimParams.bUsingRightHand) {
		EndPose.RH_Loc = TargetLocation;
	}
	else {
		EndPose.LH_Loc = TargetLocation;
	}

	// Set corresponding curves

	Curves = ReachingCurves;

	SetAnimParams(StartPose, EndPose, Curves);
}

// Start feeding animation chain
void UTaskAnimParamLogic::StartFeedingAnimChain(ACharacter *Person) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	AnimParams.bUsingLeftHand = false;
	AnimParams.bUsingRightHand = true;

	pendingStates = 2;
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunFeedingAnimChain);
	speedFactor = 0.7;

	// Set params for first animation in chain ************************
	// Approach fork to mouth

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	// Calculate EndPose

	// Person's Local Space
	FVector TargetLocation = Person->GetMesh()->GetBoneLocation(TEXT("jaw_end"), EBoneSpaces::WorldSpace);
	FVector Offset = FVector(-13, 0, -2);;

	EndPose.Head_Rot = CalculateHeadRot(TargetLocation);
	TargetLocation = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(TargetLocation);
	TargetLocation += Offset;
	EndPose.Spine_01_Rot = CalculateSpineRot(TargetLocation, 65);
	EndPose.RH_Loc = TargetLocation;

	// Set corresponding curves
	Curves = ReachingCurves;

	SetAnimParams(StartPose, EndPose, Curves);
}

// Start slicing animation chain
void UTaskAnimParamLogic::StartSlicingAnimChain(ObjectData_t ItemData, float sliceWidth) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	FVector HoldAdjusment = FVector(0, 0, 0);
	FVector EndAdjustment = FVector(0, 0, 0);

	bool orderWidthLenght = false;
	float holdingDistance = 15;
	float halfLength;
	float tmp;

	pendingStates = 2;
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunSlicingAnimChain);
	speedFactor = 0.7;

	// Set params for first animation in chain ************************
	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	// Calculate EndPose

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
		// Right Hand
		HoldAdjusment = FVector(-5, 0, 2);
		EndAdjustment = FVector(-18, 0, 5.3);

		// Left Hand
		EndPose.LH_Rot = FRotator(0, 90, -80);
	}
	else if (ItemData.Object->ActorHasTag("Steak")) {
		// Right Hand
		HoldAdjusment = FVector(-5, 6.5, 15.2);
		EndAdjustment = FVector(-20, -1, 8);
		// Left Hand
		EndPose.LH_Rot = FRotator(-30, 160, -87);
	}
	else if (ItemData.Object->ActorHasTag("Zucchini")) {
		// Right Hand
		HoldAdjusment = FVector(-7, 0, 4);
		EndAdjustment = FVector(-30, -1, 8);
		// Left Hand
		EndPose.LH_Rot = FRotator(0, 90, -80);
	}

	// local order is (Width, Lenght, Height)

	// Calculate End Point
	EndPose.RH_Loc.X = 0;
	EndPose.RH_Loc.Y = -1 * halfLength + sliceWidth;
	EndPose.RH_Loc.Z = -1 * ItemData.Extent.Z;

	// Calculate hold point
	EndPose.LH_Loc.X = 0;
	EndPose.LH_Loc.Y = halfLength - halfLength*2/3;
	EndPose.LH_Loc.Z = ItemData.Extent.Z;

	// Adjust coordinates. The IK used is controling the Avatar's wrist that shouldn't be exactly on
	// the working points but maybe a bit above and behind.
	EndPose.RH_Loc += EndAdjustment;
	EndPose.LH_Loc += HoldAdjusment;

	// Put back to original order
	if (!orderWidthLenght) {

		tmp = EndPose.RH_Loc.X;
		EndPose.RH_Loc.X = EndPose.RH_Loc.Y;
		EndPose.RH_Loc.Y = tmp;

		tmp = EndPose.LH_Loc.X;
		EndPose.LH_Loc.X = EndPose.LH_Loc.Y;
		EndPose.LH_Loc.Y = tmp;
	}

	// We are always cutting on the right extreme of the object according to Avatar's point of view.
	if (!(ItemData.angle > 354 || ItemData.angle < -354 || (ItemData.angle < 6 && ItemData.angle > -6))) {
		EndPose.RH_Loc = EndPose.RH_Loc.RotateAngleAxis(180, FVector(0, 0, 1));
		EndPose.LH_Loc = EndPose.LH_Loc.RotateAngleAxis(180, FVector(0, 0, 1));
		ItemData.angle -= 180;
	}

	// Calculate Orientation
	//LocalRot = FRotator(0, ItemData.angle, 0);

	// Convert coordinates relative to world
	EndPose.RH_Loc = ItemData.Object->GetActorTransform().TransformPosition(EndPose.RH_Loc);
	EndPose.LH_Loc = ItemData.Object->GetActorTransform().TransformPosition(EndPose.LH_Loc);

	// Convert points relative to Avatar. This is best to adjust points
	EndPose.RH_Loc = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPose.RH_Loc);
	EndPose.LH_Loc = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPose.LH_Loc);

	// Set corresponding curves
	Curves = ReachingCurves;

	SetAnimParams(StartPose, EndPose, Curves);
}

// ***************** Run animation Chains *******************

// Run passing page animation chain
void UTaskAnimParamLogic::RunPassPageAnimChain(int state) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	if (pendingStates == 2) {
		speedFactor = 1;
		Calculate_PassPage_EndPose_Curves(EndPose,Curves);
		SetAnimParams(StartPose, EndPose, Curves);
	}
	else if (pendingStates == 1) {
		speedFactor = 1;

		EndPose.Spine_01_Rot = OriginalPose.Spine_01_Rot;
		EndPose.Head_Rot = OriginalPose.Head_Rot;

		if (AnimParams.bUsingRightHand) {
			EndPose.RH_Loc = OriginalPose.RH_Loc;
			EndPose.RH_Rot = OriginalPose.RH_Rot;
			EndPose.RH_FingerRots = OriginalPose.RH_FingerRots;
		}
		else if (AnimParams.bUsingLeftHand) {
			EndPose.LH_Loc = OriginalPose.LH_Loc;
			EndPose.LH_Rot = OriginalPose.LH_Rot;
			EndPose.LH_FingerRots = OriginalPose.LH_FingerRots;
		}

		Curves = DroppingCurves;

		SetAnimParams(StartPose, EndPose, Curves, true);

	}

	pendingStates--;
	
}

// Run point book animation chain
void UTaskAnimParamLogic::RunReadNewspaperAnimChain(int state) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	float page_start = 0;
	if (AnimParams.bUsingLeftHand) page_start = -1;

	// Section 1:	- Big paragraph Middle
	if (pendingStates == 17) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0, page_start + 0.13, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Big paragraph End
	if (pendingStates == 16) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.34, page_start + 0.13, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 2 Start
	if (pendingStates == 15) {
		speedFactor = 0.5;
		Calculate_PointBook_EndPose_Curves(0, page_start + 0.29, EndPose, Curves);
		Curves = ReachingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 2 End
	if (pendingStates == 14) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.34, page_start + 0.29, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 3 Start
	if (pendingStates == 13) {
		speedFactor = 0.5;
		Calculate_PointBook_EndPose_Curves(0, page_start + 0.45, EndPose, Curves);
		Curves = ReachingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 3 End
	if (pendingStates == 12) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.34, page_start + 0.45, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}// Section 2:	- Paragraph 1 Start
	if (pendingStates == 11) {
		speedFactor = 0.5;
		Calculate_PointBook_EndPose_Curves(0, page_start + 0.62, EndPose, Curves);
		Curves = ReachingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 1 End
	if (pendingStates == 10) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.34, page_start + 0.62, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 2 Start
	if (pendingStates == 9) {
		speedFactor = 0.5;
		Calculate_PointBook_EndPose_Curves(0, page_start + 0.77, EndPose, Curves);
		Curves = ReachingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 2 End
	if (pendingStates == 8) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.34, page_start + 0.77, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}// Section 3:	- Paragraph 1 Start
	if (pendingStates == 7) {
		speedFactor = 0.5;
		Calculate_PointBook_EndPose_Curves(0.55, page_start + 0.13, EndPose, Curves);
		Curves = ReachingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 1 End
	if (pendingStates == 6) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.95, page_start + 0.13, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 2 Start
	if (pendingStates == 5) {
		speedFactor = 0.5;
		Calculate_PointBook_EndPose_Curves(0.5, page_start + 0.29, EndPose, Curves);
		Curves = ReachingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 2 End
	if (pendingStates == 4) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.95, page_start + 0.29, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 3 Start
	if (pendingStates == 3) {
		speedFactor = 0.5;
		Calculate_PointBook_EndPose_Curves(0.4, page_start + 0.45, EndPose, Curves);
		Curves = ReachingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}//				- Paragraph 3 End
	if (pendingStates == 2) {
		speedFactor = 1;
		Calculate_PointBook_EndPose_Curves(0.95, page_start + 0.45, EndPose, Curves);
		EndPose.Jaw_Rot = FRotator(0, 150, 0);
		Curves = ReadingCurves;
		SetAnimParams(StartPose, EndPose, Curves);
	}
	else if (pendingStates == 1) {
		speedFactor = 1;

		EndPose.Spine_01_Rot = OriginalPose.Spine_01_Rot;
		EndPose.Head_Rot = OriginalPose.Head_Rot;

		if (AnimParams.bUsingRightHand) {
			EndPose.RH_IndexLoc = OriginalPose.RH_IndexLoc;
			EndPose.RH_Rot = OriginalPose.RH_Rot;
			EndPose.RH_FingerRots = OriginalPose.RH_FingerRots;
		}
		else if (AnimParams.bUsingLeftHand) {
			EndPose.LH_IndexLoc = OriginalPose.LH_IndexLoc;
			EndPose.LH_Rot = OriginalPose.LH_Rot;
			EndPose.LH_FingerRots = OriginalPose.LH_FingerRots;
		}

		Curves = DroppingCurves;

		SetAnimParams(StartPose, EndPose, Curves, true);
	}
	pendingStates--;
}

// Run grasping animation chain
void UTaskAnimParamLogic::RunGraspingAnimChain(int stage) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	// Calculate EndPose

	// Grasp
	if (pendingStates == 2) {
		speedFactor = 1.5;

		FVector RH_LocEndPoint_Adjustment = FVector(0, 0, 0);
		FVector LH_LocEndPoint_Adjustment = FVector(0, 0, 0);

		if (AnimParams.Object->ActorHasTag("DinnerFork")) {
			if (AnimParams.bUsingRightHand) {
				EndPose.RH_Rot = FRotator(30, -90, 90);
				RH_LocEndPoint_Adjustment = FVector(0, 0, 5);

				// Finger's Rotations
				// Thumb
				EndPose.RH_FingerRots.thumb_01 = FRotator(-34, -20, 71);
				EndPose.RH_FingerRots.thumb_02 = FRotator(0, -46, 0);
				EndPose.RH_FingerRots.thumb_03 = FRotator(0, -23, 0);
				// Index
				EndPose.RH_FingerRots.index_01 = FRotator(0, -50, -12);
				EndPose.RH_FingerRots.index_02 = FRotator(0, -62, 0);
				EndPose.RH_FingerRots.index_03 = FRotator(0, -50, 0);
				// Middle
				EndPose.RH_FingerRots.middle_01 = FRotator(0, -60, -10);
				EndPose.RH_FingerRots.middle_02 = FRotator(0, -70, 0);
				EndPose.RH_FingerRots.middle_03 = FRotator(0, -60, 0);
				// Ring
				EndPose.RH_FingerRots.ring_01 = FRotator(0, -70, -10);
				EndPose.RH_FingerRots.ring_02 = FRotator(0, -70, 0);
				EndPose.RH_FingerRots.ring_03 = FRotator(0, -60, 0);
				// Pinky
				EndPose.RH_FingerRots.pinky_01 = FRotator(0, -90, -25);
				EndPose.RH_FingerRots.pinky_02 = FRotator(0, -60, 0);
				EndPose.RH_FingerRots.pinky_03 = FRotator(0, -60, 0);
			}
		}
		else if (AnimParams.Object->ActorHasTag("Plate")) {
			if (AnimParams.bUsingRightHand) {
				EndPose.RH_Rot = FRotator(45, -135, -45);
				RH_LocEndPoint_Adjustment = FVector(-15, -2.727218, 4.5);

				EndPose.RH_FingerRots.thumb_01 = FRotator(10, -15, 130);
				EndPose.RH_FingerRots.index_01 = FRotator(0, -35, -12);
				EndPose.RH_FingerRots.middle_01 = FRotator(0, -45, -10);
				EndPose.RH_FingerRots.ring_01 = FRotator(0, -55, -10);
				EndPose.RH_FingerRots.pinky_01 = FRotator(0, -60, -25);

				EndPose.RH_FingerRots.thumb_02 = FRotator(0, 0, 0);
				EndPose.RH_FingerRots.index_02 = FRotator(0, -20, 0);
				EndPose.RH_FingerRots.middle_02 = FRotator(0, -15, 0);
				EndPose.RH_FingerRots.ring_02 = FRotator(0, -10, 0);
				EndPose.RH_FingerRots.pinky_02 = FRotator(0, -10, 0);

				EndPose.RH_FingerRots.thumb_03 = FRotator(0, 0, 0);
				EndPose.RH_FingerRots.index_03 = FRotator(0, -10, 0);
				EndPose.RH_FingerRots.middle_03 = FRotator(0, -10, 0);
				EndPose.RH_FingerRots.ring_03 = FRotator(0, -5, 0);
				EndPose.RH_FingerRots.pinky_03 = FRotator(0, -10, 0);
			}
		}
		else if (AnimParams.Object->ActorHasTag("BreadKnife")) {
			if (AnimParams.bUsingRightHand) {
				EndPose.RH_Rot = FRotator(10, -30, 90);
				RH_LocEndPoint_Adjustment = FVector(5, -13, 5);

				// Fingers' Rotations
				// Thumb
				EndPose.RH_FingerRots.thumb_01 = FRotator(-45, -48, 90);
				EndPose.RH_FingerRots.thumb_02 = FRotator(25, -55, 0);
				EndPose.RH_FingerRots.thumb_03 = FRotator(0, -45, 0);
				// Index
				EndPose.RH_FingerRots.index_01 = FRotator(-3, -80, 0);
				EndPose.RH_FingerRots.index_02 = FRotator(0, -90, 0);
				EndPose.RH_FingerRots.index_03 = FRotator(0, -45, 0);
				// Middle
				EndPose.RH_FingerRots.middle_01 = FRotator(0, -90, 0);
				EndPose.RH_FingerRots.middle_02 = FRotator(0, -90, 0);
				EndPose.RH_FingerRots.middle_03 = FRotator(0, -45, 0);
				// Ring
				EndPose.RH_FingerRots.ring_01 = FRotator(3, -90, 0);
				EndPose.RH_FingerRots.ring_02 = FRotator(0, -90, 0);
				EndPose.RH_FingerRots.ring_03 = FRotator(0, -45, 0);
				// Pinky
				EndPose.RH_FingerRots.pinky_01 = FRotator(5, -100, 0);
				EndPose.RH_FingerRots.pinky_02 = FRotator(0, -80, 10);
				EndPose.RH_FingerRots.pinky_03 = FRotator(0, -45, 0);
			}
		}

		FVector ObjectLoc = AnimParams.Object->GetActorLocation();
		EndPose.Head_Rot = CalculateHeadRot(ObjectLoc);

		if (AnimParams.bUsingLeftHand) {
			EndPose.LH_Loc = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(ObjectLoc);
			EndPose.LH_Loc += LH_LocEndPoint_Adjustment;
			EndPose.Spine_01_Rot = CalculateSpineRot(EndPose.LH_Loc, 65);
		}
		if (AnimParams.bUsingRightHand) {
			EndPose.RH_Loc = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(ObjectLoc);
			EndPose.RH_Loc += RH_LocEndPoint_Adjustment;
			EndPose.Spine_01_Rot = CalculateSpineRot(EndPose.RH_Loc, 65);
		}
	}
	else if (pendingStates == 3) {
		// 1 Seconds Delay
		AnimParams.AnimFunctionDelegate.Unbind();
		speedFactor = 1;
		AnimParams.animTime = 1;
		bRunAnimation = true;
	}
	else if (pendingStates == 1) {
		// Attach
		AttachObject();
		speedFactor = 0.7;

		if (AnimParams.Object->ActorHasTag("BreadKnife")) {
			if (AnimParams.bUsingRightHand) {
				EndPose.RH_Rot = FRotator(45, -145, 35);
				EndPose.RH_Loc = FVector(-15, 15, 100);
			}
		}
		else if (AnimParams.Object->ActorHasTag("DinnerFork")) {
			if (AnimParams.bUsingRightHand) {
				EndPose.RH_Rot = FRotator(9, -100, -25);
				EndPose.RH_Loc = FVector(-15, 15, 100);
			}
		}
		else if (AnimParams.Object->ActorHasTag("Plate")) {
			if (AnimParams.bUsingRightHand) {
				EndPose.RH_Loc = FVector(-15, 20, 100);
			}
		}
		else {
			if (AnimParams.bUsingRightHand) {
				EndPose.RH_Loc = FVector(-15, 15, 100);
				EndPose.RH_Rot = FRotator(45, -110, 60);
			}
			if (AnimParams.bUsingLeftHand) {
				EndPose.RH_Loc = FVector(15, 15, 100);
				EndPose.RH_Rot = FRotator(45, -110, 60);
			}
		}

		if (AnimParams.bUsingLeftHand) {
			EndPose.Spine_01_Rot = CalculateSpineRot(EndPose.LH_Loc, 65);
		}
		if (AnimParams.bUsingRightHand) {
			EndPose.Spine_01_Rot = CalculateSpineRot(EndPose.RH_Loc, 65);
		}

		EndPose.Head_Rot = FRotator(0, 0, 0);
	}

	// Set corresponding curves
	Curves = ReachingCurves;

	SetAnimParams(StartPose, EndPose, Curves);
	pendingStates--;
}

// Run placing animation chain
void UTaskAnimParamLogic::RunPlacingAnimChain(int stage) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	if (pendingStates == 1) {

		speedFactor = 1;

		// Dettach
		if (AnimParams.bUsingRightHand) {
			Avatar->DetachGraspedObject_r();
		}
		else {
			Avatar->DetachGraspedObject_l();
		}

		// Calculate EndPose
		EndPose.Spine_01_Rot = OriginalPose.Spine_01_Rot;
		EndPose.Head_Rot = OriginalPose.Head_Rot;

		if (AnimParams.bUsingRightHand) {
			EndPose.RH_Loc = OriginalPose.RH_Loc;
			EndPose.RH_Rot = OriginalPose.RH_Rot;
			EndPose.RH_FingerRots = OriginalPose.RH_FingerRots;
		}
		else if (AnimParams.bUsingLeftHand) {
			EndPose.LH_Loc = OriginalPose.LH_Loc;
			EndPose.LH_Rot = OriginalPose.LH_Rot;
			EndPose.LH_FingerRots = OriginalPose.LH_FingerRots;
		}

		// Set corresponding curves
		Curves = DroppingCurves;

		SetAnimParams(StartPose, EndPose, Curves, true);
	}

	pendingStates--;
}

// Run feeding animation chain
void UTaskAnimParamLogic::RunFeedingAnimChain(int stage) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	if (pendingStates == 2) {
		// 1.5 Seconds Delay
		AnimParams.ActionContext = "waiting with fork";
		AnimParams.AnimFunctionDelegate.Unbind();
		speedFactor = 1;
		AnimParams.animTime = 1.5;
		bRunAnimation = true;
	}
	else if (pendingStates == 1) {
	
		speedFactor = 1;

		// Calculate EndPose
		if (AnimParams.bUsingRightHand) {
			EndPose.RH_Loc = FVector(-15, 15, 100);
		}
		else {
			EndPose.LH_Loc = FVector(15, 15, 100);
		}

		EndPose.Spine_01_Rot = FRotator(0, 0, 0);

		// Set corresponding curves
		Curves = ReachingCurves;

		SetAnimParams(StartPose, EndPose, Curves);
	}

	pendingStates--;
}

// Run slicing animation chain
void UTaskAnimParamLogic::RunSlicingAnimChain(int stage) {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	if (pendingStates == 2) {
		// 1.5 Seconds Delay
		AnimParams.ActionContext = "waiting with fork";
		AnimParams.AnimFunctionDelegate.Unbind();
		speedFactor = 1;
		AnimParams.animTime = 1.5;
		bRunAnimation = true;
	}
	else if (pendingStates == 1) {
		
		StartPose = GetCurrentAvatarPose();
		EndPose = StartPose;

		EndPose.LH_Loc = FVector(15, 15, 100);
		EndPose.RH_Loc = FVector(-15, 15, 100);

		Curves = ReachingCurves;

		SetAnimParams(StartPose, EndPose, Curves);
		speedFactor = 1;
	}

	pendingStates--;
}

// ********** Set parameters for tasks' animation **********

// General anim params setter
void UTaskAnimParamLogic::SetAnimParams(FAvatarPose_t StartPose, FAvatarPose_t EndPose, FCurvesSet_t Curves, bool bReleaseAlphas) {

	FAvatarPose_t MultiplierPose;

	// Multiplier
	MultiplierPose.RH_Loc = EndPose.RH_Loc - StartPose.RH_Loc;
	MultiplierPose.LH_Loc = EndPose.LH_Loc - StartPose.LH_Loc;

	MultiplierPose.RH_Rot = EndPose.RH_Rot - StartPose.RH_Rot;
	MultiplierPose.LH_Rot = EndPose.LH_Rot - StartPose.LH_Rot;

	MultiplierPose.RH_IndexLoc = EndPose.RH_IndexLoc - StartPose.RH_IndexLoc;
	MultiplierPose.LH_IndexLoc = EndPose.LH_IndexLoc - StartPose.LH_IndexLoc;

	MultiplierPose.RH_FingerRots = EndPose.RH_FingerRots - StartPose.RH_FingerRots;
	MultiplierPose.LH_FingerRots = EndPose.LH_FingerRots - StartPose.LH_FingerRots;

	MultiplierPose.Spine_01_Rot = EndPose.Spine_01_Rot - StartPose.Spine_01_Rot;
	MultiplierPose.Head_Rot = EndPose.Head_Rot - StartPose.Head_Rot;
	MultiplierPose.Jaw_Rot = EndPose.Jaw_Rot - StartPose.Jaw_Rot;

	// Set AnimParams
	if (!MultiplierPose.RH_Loc.IsZero()) {
		AnimParams.RH_Loc_Offset = StartPose.RH_Loc;
		AnimParams.RH_Loc_Multiplier = MultiplierPose.RH_Loc;
		AnimParams.RH_Loc_Curve = Curves.RH_Loc_Interpolation;
		AnimParams.bSet_RH_Loc = true;
		if (bReleaseAlphas) AnimParams.bUnSet_RH_Loc = true;
	}
	if (!MultiplierPose.RH_Rot.IsZero()) {
		AnimParams.RH_Rot_Offset = StartPose.RH_Rot;
		AnimParams.RH_Rot_Multiplier = MultiplierPose.RH_Rot;
		AnimParams.RH_Rot_Curve = Curves.RH_Rot_Interpolation;
		AnimParams.bSet_RH_Rot = true;
		if (bReleaseAlphas) AnimParams.bUnSet_RH_Rot = true;
	}
	if (!MultiplierPose.RH_FingerRots.IsZero()) {
		AnimParams.RH_FingerRots_Offset = StartPose.RH_FingerRots;
		AnimParams.RH_FingerRots_Multiplier = MultiplierPose.RH_FingerRots;
		AnimParams.RH_FingerRots_Curve = Curves.RH_FingerRots_Interpolation;
		AnimParams.bSet_RF_Rot = true;
		if (bReleaseAlphas) AnimParams.bUnSet_RF_Rot = true;
	}
	if (!MultiplierPose.LH_Loc.IsZero()) {
		AnimParams.LH_Loc_Offset = StartPose.LH_Loc;
		AnimParams.LH_Loc_Multiplier = MultiplierPose.LH_Loc;
		AnimParams.LH_Loc_Curve = Curves.LH_Loc_Interpolation;
		AnimParams.bSet_LH_Loc = true;
		if (bReleaseAlphas) AnimParams.bUnSet_LH_Loc = true;
	}
	if (!MultiplierPose.LH_Rot.IsZero()) {
		AnimParams.LH_Rot_Offset = StartPose.LH_Rot;
		AnimParams.LH_Rot_Multiplier = MultiplierPose.LH_Rot;
		AnimParams.LH_Rot_Curve = Curves.LH_Rot_Interpolation;
		AnimParams.bSet_LH_Rot = true;
		if (bReleaseAlphas) AnimParams.bUnSet_LH_Rot = true;
	}
	if (!MultiplierPose.LH_FingerRots.IsZero()) {
		AnimParams.LH_FingerRots_Offset = StartPose.LH_FingerRots;
		AnimParams.LH_FingerRots_Multiplier = MultiplierPose.LH_FingerRots;
		AnimParams.LH_FingerRots_Curve = Curves.LH_FingerRots_Interpolation;
		AnimParams.bSet_LF_Rot = true;
		if (bReleaseAlphas) AnimParams.bUnSet_LF_Rot = true;
	}
	if (!MultiplierPose.RH_IndexLoc.IsZero()) {
		AnimParams.RH_IndexLoc_Offset = StartPose.RH_IndexLoc;
		AnimParams.RH_IndexLoc_Multiplier = MultiplierPose.RH_IndexLoc;
		AnimParams.RH_IndexLoc_Curve = Curves.RH_IndexLoc_Interpolation;
		AnimParams.bSet_RI_Loc = true;
		if (bReleaseAlphas) AnimParams.bUnSet_RI_Loc = true;
	}
	if (!MultiplierPose.LH_IndexLoc.IsZero()) {
		AnimParams.LH_IndexLoc_Offset = StartPose.LH_IndexLoc;
		AnimParams.LH_IndexLoc_Multiplier = MultiplierPose.LH_IndexLoc;
		AnimParams.LH_IndexLoc_Curve = Curves.LH_IndexLoc_Interpolation;
		AnimParams.bSet_LI_Loc = true;
		if (bReleaseAlphas) AnimParams.bUnSet_LI_Loc = true;
	}
	if (!MultiplierPose.Spine_01_Rot.IsZero()) {
		AnimParams.Spine_01_Rot_Offset = StartPose.Spine_01_Rot;
		AnimParams.Spine_01_Rot_Multiplier = MultiplierPose.Spine_01_Rot;
		AnimParams.Spine_01_Rot_Curve = Curves.Spine_01_Rot_Interpolation;
		AnimParams.bSet_S01_Rot = true;
		if (bReleaseAlphas) AnimParams.bUnSet_S01_Rot= true;
	}
	if (!MultiplierPose.Head_Rot.IsZero()) {
		AnimParams.Head_Rot_Offset = StartPose.Head_Rot;
		AnimParams.Head_Rot_Multiplier = MultiplierPose.Head_Rot;
		AnimParams.Head_Rot_Curve = Curves.Head_Rot_Interpolation;
		AnimParams.bSet_Head_Rot = true;
		if (bReleaseAlphas) AnimParams.bUnSet_Head_Rot = true;
	}

	if (!MultiplierPose.Jaw_Rot.IsZero()) {
		AnimParams.Jaw_Rot_Offset = StartPose.Jaw_Rot;
		AnimParams.Jaw_Rot_Multiplier = MultiplierPose.Jaw_Rot;
		AnimParams.Jaw_Rot_Curve = Curves.Jaw_Rot_Interpolation;
		AnimParams.bSet_Jaw_Rot = true;
		if (bReleaseAlphas) AnimParams.bUnSet_Jaw_Rot = true;
	}

	AnimParams.animTime = Curves.time;
	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunAnimation);
	bRunAnimation = true;
}

#pragma optimize("", off)
void UTaskAnimParamLogic::StartCutAnimation(ObjectData_t &ItemData) {

	bool orderWidthLenght = false;
	FVector HoldPoint;
	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier = FVector(0, 0, 0);
	FVector HoldAdjusment = FVector(0, 0, 0);
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
		AnimParams.RH_Loc_Curve = CuttingBreadCurves.RH_Loc_Interpolation;
		AnimParams.RH_Rot_Curve = CuttingBreadCurves.RH_Rot_Interpolation;
		AnimParams.LH_Loc_Curve = CuttingBreadCurves.LH_Loc_Interpolation;
		AnimParams.LH_Rot_Curve = CuttingBreadCurves.LH_Rot_Interpolation;
		AnimParams.Spine_01_Rot_Curve = CuttingBreadCurves.Spine_01_Rot_Interpolation;

		AnimParams.RH_Loc_Table = new DataTableHandler(
			FPaths::ProjectPluginsDir() + FString("UIAIAvatar/Content/Animation_Assets/AnimDataTables/CuttingBread/RH_Curve.csv"));
		AnimParams.RH_Rot_Table = new DataTableHandler(
			FPaths::ProjectPluginsDir() + FString("UIAIAvatar/Content/Animation_Assets/AnimDataTables/CuttingBread/RH_Rot_Curve.csv"));
		AnimParams.LH_Loc_Table = new DataTableHandler(
			FPaths::ProjectPluginsDir() + FString("UIAIAvatar/Content/Animation_Assets/AnimDataTables/CuttingBread/LH_Curve.csv"));
		AnimParams.LH_Rot_Table = new DataTableHandler(
			FPaths::ProjectPluginsDir() + FString("UIAIAvatar/Content/Animation_Assets/AnimDataTables/CuttingBread/LH_Rot_Curve.csv"));
		AnimParams.Spine_01_Rot_Table = new DataTableHandler(
			FPaths::ProjectPluginsDir() + FString("UIAIAvatar/Content/Animation_Assets/AnimDataTables/CuttingBread/Spine01_Rot_Curve.csv"));
	
		AnimParams.animTime = AnimParams.RH_Rot_Table->GetDuration();
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
		AnimParams.RH_Loc_Curve = CuttingSteakCurves.RH_Loc_Interpolation;
		AnimParams.RH_Rot_Curve = CuttingSteakCurves.RH_Rot_Interpolation;
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
		AnimParams.RH_Loc_Curve = CuttingZucchiniCurves.RH_Loc_Interpolation;
		AnimParams.RH_Rot_Curve = CuttingZucchiniCurves.RH_Rot_Interpolation;
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
	EndPoint = ItemData.Object->GetActorTransform().TransformPosition(EndPoint);
	HoldPoint = ItemData.Object->GetActorTransform().TransformPosition(HoldPoint);

	// Convert points relative to Avatar. This is best to adjust points
	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);
	HoldPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(HoldPoint);

	// Start Point is holding position
	StartPoint = Avatar->GetMesh()->GetBoneLocation("hand_r", EBoneSpaces::ComponentSpace); 

	// Motion Depth
	Multiplier = EndPoint - StartPoint;

	// Set animation parameters
	AnimParams.RH_Loc_Offset = StartPoint;
	AnimParams.RH_Loc_Multiplier = Multiplier;
	AnimParams.LH_Rot_Multiplier = HoldRotation;
	AnimParams.LH_Loc_Offset = Avatar->GetMesh()->GetBoneLocation("hand_l", EBoneSpaces::ComponentSpace);
	AnimParams.LH_Loc_Multiplier = HoldPoint - AnimParams.LH_Loc_Offset;

	AnimParams.RH_Loc_Curve_Orientation = LocalRot;

	AnimParams.bSet_LH_Loc = true;
	AnimParams.bSet_LH_Rot = true;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;

	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunCutAnimation);
	bRunAnimation = true;

}
#pragma optimize("", on)

void UTaskAnimParamLogic::StartForkAnimation(AActor* Target) {

	// Hand
	FVector LocStartPoint;
	FVector LocEndPoint;
	FVector LocMultiplier;

	FRotator RotStartPoint;
	FRotator RotEndPoint;
	FRotator RotMultiplier;

	// Spine
	FRotator SpineRotStartPoint;
	FRotator SpineRotEndPoint;
	FRotator SpineRotMultiplier;

	// Head 
	FRotator HeadRotStartPoint;
	FRotator HeadRotEndPoint;
	FRotator HeadRotMultiplier;

	// Adjusments
	FVector EndAdjustment = FVector(-7, -7, 15);

	AnimParams.Object = Target;
	AnimParams.ClearJointFlags();
	AnimParams.bUsingRightHand = false;
	AnimParams.bUsingLeftHand = false;

	// StartPoints
	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	SpineRotStartPoint = TempRotOut;
	SpineRotEndPoint = OriginalPose.Spine_01_Rot;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	HeadRotStartPoint = TempRotOut;
	HeadRotEndPoint = OriginalPose.Head_Rot;

	LocStartPoint = Avatar->GetMesh()->GetBoneLocation("hand_r", EBoneSpaces::ComponentSpace);
	RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();
	
	// Endpoints 

	// Head
	LocEndPoint = Target->GetActorLocation();
	HeadRotEndPoint = CalculateHeadRot(LocEndPoint);

	// Hand Loc
	LocEndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(LocEndPoint);
	LocEndPoint += EndAdjustment;

	// Hand Rot
	RotEndPoint = FRotator(9,-90,45);

	//Spine
	SpineRotEndPoint = CalculateSpineRot(LocEndPoint,65);

	// Multipliers
	LocMultiplier = LocEndPoint - LocStartPoint;
	RotMultiplier = RotEndPoint - RotStartPoint;
	SpineRotMultiplier = SpineRotEndPoint - SpineRotStartPoint;
	HeadRotMultiplier = HeadRotEndPoint - HeadRotStartPoint;

	// Parameters
	AnimParams.RH_Loc_Offset = LocStartPoint;
	AnimParams.RH_Loc_Multiplier = LocMultiplier;
	AnimParams.RH_Loc_Curve = ForkingCurves.RH_Loc_Interpolation;

	AnimParams.RH_Rot_Offset = RotStartPoint;
	AnimParams.RH_Rot_Multiplier = RotMultiplier;
	AnimParams.RH_Rot_Curve = ForkingCurves.RH_Rot_Interpolation;

	AnimParams.Spine_01_Rot_Offset = SpineRotStartPoint;
	AnimParams.Spine_01_Rot_Multiplier = SpineRotMultiplier;
	AnimParams.Spine_01_Rot_Curve = ForkingCurves.Spine_01_Rot_Interpolation;

	AnimParams.Head_Rot_Offset = HeadRotStartPoint;
	AnimParams.Head_Rot_Multiplier = HeadRotMultiplier;
	AnimParams.Head_Rot_Curve = ForkingCurves.Head_Rot_Interpolation;

	AnimParams.animTime = 4.5;

	AnimParams.bUsingRightHand = true;
	AnimParams.bSet_RH_Loc     = true;
	AnimParams.bSet_RH_Rot     = true;
	AnimParams.bSet_S01_Rot    = true;
	AnimParams.bSet_Head_Rot   = true;

	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunForkAnimation);
	bRunAnimation = true;

}	

void UTaskAnimParamLogic::StartSpoonAnimation(AActor* Target) {

	FVector Origin;
	FVector Extent;
	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier;

	FVector EndAdjustment = FVector(-13, -7, 9);
	FVector StartAdjustment = FVector(0, 0, 0);

	AnimParams.Spine_01_Rot_Multiplier = FRotator(0, 20, 0);

	// Get origin and extension							
	Target->GetActorBounds(false, Origin, Extent);

	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Origin);
	EndPoint += EndAdjustment;

	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = (EndPoint - StartPoint);

	//
	AnimParams.RH_Loc_Offset = StartPoint;
	AnimParams.RH_Loc_Multiplier = Multiplier;

	// Skill
	AnimParams.RH_Loc_Curve = SpooningSoupCurves.RH_Loc_Interpolation;
	AnimParams.RH_Rot_Curve = SpooningSoupCurves.RH_Rot_Interpolation ;

	AnimParams.animTime = 5;

	AnimParams.bSet_LH_Loc = false;
	AnimParams.bSet_LH_Rot = false;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;

	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunSpoonAnimation);
	bRunAnimation = true;

}

void UTaskAnimParamLogic::StartPourAnimation(AActor* Target) {

	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier;
	FVector EndAdjustment = FVector(-15, -5, 18);

	AnimParams.Spine_01_Rot_Multiplier = FRotator(0, 15, 0);

	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Target->GetActorLocation());
	EndPoint += EndAdjustment;

	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = (EndPoint - StartPoint);

	//
	AnimParams.RH_Loc_Offset = StartPoint;
	AnimParams.RH_Loc_Multiplier = Multiplier;

	// Skill
	AnimParams.RH_Loc_Curve = PouringCurves.RH_Loc_Interpolation;
	AnimParams.RH_Rot_Curve = PouringCurves.RH_Rot_Interpolation;

	AnimParams.animTime = 9;

	AnimParams.bSet_LH_Loc = false;
	AnimParams.bSet_LH_Rot = false;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;

	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunPourAnimation);
	bRunAnimation = true;

}

void UTaskAnimParamLogic::StartLookAnimation(FVector Point) {
	
	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	AnimParams.bUsingLeftHand = false;
	AnimParams.bUsingRightHand = false;
	speedFactor = 2;

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	EndPose.Head_Rot = CalculateHeadRot(Point);

	Curves.time = 1.25;
	Curves.Head_Rot_Interpolation = ReachingCurves.Head_Rot_Interpolation;

	SetAnimParams(StartPose, EndPose, Curves);
}

void UTaskAnimParamLogic::StartReleaseLookAnimation() {

	FAvatarPose_t StartPose, EndPose, MultiplierPose;
	FCurvesSet_t Curves;

	AnimParams.ClearJointFlags();
	AnimParams.bUsingLeftHand = false;
	AnimParams.bUsingRightHand = false;
	speedFactor = 2;

	StartPose = GetCurrentAvatarPose();
	EndPose = StartPose;

	EndPose.Head_Rot = FRotator(0,0,0);

	Curves.time = 1.25;
	Curves.Head_Rot_Interpolation = ReachingCurves.Head_Rot_Interpolation;

	SetAnimParams(StartPose, EndPose, Curves, true);
}

// ******************* Run Animations ************************

// General animation runner
void UTaskAnimParamLogic::RunAnimation(float time) {

	FVector TempVec;
	FRotator TempRot;
	FFingerRots_t TempFingerRots;

	// Right Hand Rotation
	if (AnimParams.bSet_RH_Rot) {
		TempVec = AnimParams.RH_Rot_Curve->GetVectorValue(time);
		TempRot = AnimParams.RH_Rot_Multiplier;
		TempRot.Pitch *= TempVec.Y;
		TempRot.Yaw *= TempVec.Z;
		TempRot.Roll *= TempVec.X;
		TempRot += AnimParams.RH_Rot_Offset;
		Animation->RightHandRotation = TempRot;
	}

	// Right Hand Location
	if (AnimParams.bSet_RH_Loc) {
		TempVec = AnimParams.RH_Loc_Curve->GetVectorValue(time);
		TempVec *= AnimParams.RH_Loc_Multiplier;
		TempVec += AnimParams.RH_Loc_Offset;
		Animation->RightHandIKTargetPosition = TempVec;
	}

	// Right Hand Finger Rotations
	if (AnimParams.bSet_RF_Rot) {
		TempFingerRots = AnimParams.RH_FingerRots_Multiplier;
		TempFingerRots *= AnimParams.RH_FingerRots_Curve->GetFloatValue(time);
		TempFingerRots += AnimParams.RH_FingerRots_Offset;
		Animation->RightHandFingerRots = TempFingerRots;
	}

	// Right Hand Index Location
	if (AnimParams.bSet_RI_Loc) {
		TempVec = AnimParams.RH_IndexLoc_Curve->GetVectorValue(time);
		TempVec *= AnimParams.RH_IndexLoc_Multiplier;
		TempVec += AnimParams.RH_IndexLoc_Offset;
		Animation->RightFingerIKTargetPosition = TempVec;
	}

	// Left Hand Rotation
	if (AnimParams.bSet_LH_Rot) {
		TempVec = AnimParams.LH_Rot_Curve->GetVectorValue(time);
		TempRot = AnimParams.LH_Rot_Multiplier;
		TempRot.Pitch *= TempVec.Y;
		TempRot.Yaw *= TempVec.Z;
		TempRot.Roll *= TempVec.X;
		TempRot += AnimParams.LH_Rot_Offset;
		Animation->HandRotation = TempRot;
	}

	// Left Hand Location
	if (AnimParams.bSet_LH_Loc) {
		TempVec = AnimParams.LH_Loc_Curve->GetVectorValue(time);
		TempVec *= AnimParams.LH_Loc_Multiplier;
		TempVec += AnimParams.LH_Loc_Offset;
		Animation->LeftHandIKTargetPosition = TempVec;
	}

	// Left Hand Finger Rotations
	if (AnimParams.bSet_LF_Rot) {
		TempFingerRots = AnimParams.LH_FingerRots_Multiplier;
		TempFingerRots *= AnimParams.LH_FingerRots_Curve->GetFloatValue(time);
		TempFingerRots += AnimParams.LH_FingerRots_Offset;
		Animation->LeftHandFingerRots = TempFingerRots;
	}

	// Left Hand Index Location
	if (AnimParams.bSet_LI_Loc) {
		TempVec = AnimParams.LH_IndexLoc_Curve->GetVectorValue(time);
		TempVec *= AnimParams.LH_IndexLoc_Multiplier;
		TempVec += AnimParams.LH_IndexLoc_Offset;
		Animation->LeftFingerIKTargetPosition = TempVec;
	}

	// Spine Rotation
	if (AnimParams.bSet_S01_Rot) {
		TempVec = AnimParams.Spine_01_Rot_Curve->GetVectorValue(time);
		TempRot = FRotator(TempVec.Y, TempVec.Z, TempVec.X);
		TempRot.Pitch *= AnimParams.Spine_01_Rot_Multiplier.Pitch;
		TempRot.Yaw *= AnimParams.Spine_01_Rot_Multiplier.Yaw;
		TempRot.Roll *= AnimParams.Spine_01_Rot_Multiplier.Roll;
		TempRot += AnimParams.Spine_01_Rot_Offset;
		Animation->Spine1Rotation = TempRot;
	}

	// Head Rotation
	if (AnimParams.bSet_Head_Rot) {
		TempVec = AnimParams.Head_Rot_Curve->GetVectorValue(time);
		TempRot = FRotator(TempVec.Y, TempVec.Z, TempVec.X);
		TempRot.Pitch *= AnimParams.Head_Rot_Multiplier.Pitch;
		TempRot.Yaw *= AnimParams.Head_Rot_Multiplier.Yaw;
		TempRot.Roll *= AnimParams.Head_Rot_Multiplier.Roll;
		TempRot += AnimParams.Head_Rot_Offset;
		Animation->HeadRotation = TempRot;
	}

	//Jaw Rotation
	if (AnimParams.bSet_Jaw_Rot) {
		TempVec = AnimParams.Jaw_Rot_Curve->GetVectorValue(time);
		TempRot = FRotator(TempVec.Y, TempVec.Z, TempVec.X);
		TempRot.Pitch *= AnimParams.Jaw_Rot_Multiplier.Pitch;
		TempRot.Yaw *= AnimParams.Jaw_Rot_Multiplier.Yaw;
		TempRot.Roll *= AnimParams.Jaw_Rot_Multiplier.Roll;
		TempRot += AnimParams.Jaw_Rot_Offset;
		Animation->JawRotation = TempRot;
	}
}

// TODO: Separate slicing animation from reaching animation. Reach in terms to right before cutting. 
// Run cut animation
void UTaskAnimParamLogic::RunCutAnimation(float time) {

	FVector Temp;
	FVector Hand_r_Location;
	FVector Hand_l_Location;
	FRotator Hand_r_Rotation;
	FRotator Hand_l_Rotation;

	// Right hand rotation
	//Temp = AnimParams.RH_Rot_Curve->GetVectorValue(time);
	Temp = AnimParams.RH_Rot_Table->GetVectorValue(time);
	Hand_r_Rotation = FRotator(Temp.Y, Temp.Z, Temp.X);
	Hand_r_Rotation += AnimParams.RH_Loc_Curve_Orientation;

	// Right hand location
	//Hand_r_Location = AnimParams.RH_Loc_Curve->GetVectorValue(time);
	Hand_r_Location = AnimParams.RH_Loc_Table->GetVectorValue(time);
	Hand_r_Location *= AnimParams.RH_Loc_Multiplier;
	//Hand_r_Location = AnimParams.RH_Loc_Curve_Orientation.RotateVector(Hand_r_Location);
	Hand_r_Location += AnimParams.RH_Loc_Offset;

	// Left hand rotation
	//Temp = AnimParams.LH_Rot_Curve->GetVectorValue(time);
	Temp = AnimParams.LH_Rot_Table->GetVectorValue(time);
	Hand_l_Rotation = FRotator(Temp.Y, Temp.Z, Temp.X);

	// Left hand location
	//Hand_l_Location = AnimParams.LH_Loc_Curve->GetVectorValue(time);
	Hand_l_Location = AnimParams.LH_Loc_Table->GetVectorValue(time);
	Hand_l_Location *= AnimParams.LH_Loc_Multiplier;
	Hand_l_Location += AnimParams.LH_Loc_Offset;

	// Assigning on animation

	// Right hand
	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;

	// Left hand
	Animation->HandRotation = Hand_l_Rotation;
	Animation->LeftHandIKTargetPosition = Hand_l_Location;
	
	// Spine
	//Temp = AnimParams.Spine01_Rot_Curve->GetVectorValue(time);
	Temp = AnimParams.Spine_01_Rot_Table->GetVectorValue(time);
	Animation->Spine1Rotation = FRotator(Temp.Y, Temp.Z, Temp.X);
}

// Run fork animation
void UTaskAnimParamLogic::RunForkAnimation(float time) {

	bool static attached = false;

	if (time == 0) {
		attached = false;
	}
	
	RunAnimation(time);

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
	Hand_r_Location *= AnimParams.RH_Loc_Multiplier;
	Hand_r_Location += AnimParams.RH_Loc_Offset;

	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;
	Animation->Spine1Rotation = AnimParams.Spine_01_Rot_Multiplier;
}

// Run fork animation
void UTaskAnimParamLogic::RunPourAnimation(float time) {

	FVector Temp;
	FVector Hand_r_Location;
	FRotator Hand_r_Rotation;

	Temp = AnimParams.RH_Rot_Curve->GetVectorValue(time);
	Hand_r_Rotation = FRotator(Temp.Y, Temp.Z, Temp.X);

	Hand_r_Location = AnimParams.RH_Loc_Curve->GetVectorValue(time);
	Hand_r_Location *= AnimParams.RH_Loc_Multiplier;
	Hand_r_Location += AnimParams.RH_Loc_Offset;

	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;
	Animation->Spine1Rotation = AnimParams.Spine_01_Rot_Multiplier;
}


// ******************* Call Animations ***********************

// Spooning
void UTaskAnimParamLogic::CallSpoonAnimation(FString ObjectName) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {
		StartSpoonAnimation(Object);
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Forking
void UTaskAnimParamLogic::CallForkAnimation(FString ObjectName) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {
		StartForkAnimation(Object);
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Pouring
void UTaskAnimParamLogic::CallPourAnimation(FString ObjectName) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {
		StartPourAnimation(Object);
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Poiting book
void UTaskAnimParamLogic::CallPointBookAnimChain(FString ObjectName) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {
		if (Object->ActorHasTag("book")) {
			StartReadNewspaperAnimChain(Object, "left");
		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided as book."));
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Passing page
void UTaskAnimParamLogic::CallPassPageAnimChain(FString ObjectName) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {
		if (Object->ActorHasTag("book")) {
			StartPassPageAnimChain(Object);
		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided as book."));
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Closing book
void UTaskAnimParamLogic::CallCloseBookAnimChain(FString ObjectName) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {
		if (Object->ActorHasTag("book")) {
			StartPassPageAnimChain(Object, true);
		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided as book."));
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Slicing
void UTaskAnimParamLogic::CallSlicingAnimChain(FString ObjectName, float width) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {
		ObjectData_t CurrentCutable;
		if (Object->GetRootComponent()->ComponentHasTag("Cuttable")) {
			CurrentCutable.Object = Object;
			if (isInGoodAlignment(CurrentCutable)) {
				StartSlicingAnimChain(CurrentCutable, width);
			}
		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided as cuttable."));
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Grasping object
void UTaskAnimParamLogic::CallGraspingAnimChain(FString ObjectName, FString Hand, bool bHold) {

	TMap<FString, FHitResult> MyUniqueHits = Avatar->ListObjects();
	AActor *Object = MyUniqueHits.FindRef(ObjectName).GetActor();

	if (Object != NULL) {
		StartGraspingAnimChain(Object, Hand, bHold);
	}
	else {
		UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Object \"%s\" not found!"), *ObjectName);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: Object \"%s\" not found!"), *ObjectName), true, FVector2D(1.7, 1.7));
		return;
	}
}

// Placing object
void UTaskAnimParamLogic::CallPlacingAnimChain(FString ObjectName) {
	AActor * Object = CheckForObject(Avatar->ListObjects(), ObjectName);
	if (Object != NULL) {

	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

// Feeding person
void UTaskAnimParamLogic::CallFeedingAnimChain(FString PersonName) {

	TMap<FString, FHitResult> MyUniqueHits = Avatar->ListObjects();
	AActor *Actor = MyUniqueHits.FindRef(PersonName).GetActor();

	if (Actor == NULL) {
		UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Actor \"%s\" not found!"), *PersonName);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: Actor \"%s\" not found!"), *PersonName), true, FVector2D(1.7, 1.7));
		return;
	}

	ACharacter *Person = Cast<ACharacter>(Actor);

	if (Person == NULL) {
		UE_LOG(LogAvatarCharacter, Log, TEXT("ERROR: Actor \"%s\" is not a character!"), *PersonName);
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Red, FString::Printf(TEXT("ERROR: Actor \"%s\" is not a character!"), *PersonName), true, FVector2D(1.7, 1.7));
		return;
	}

	StartFeedingAnimChain(Person);
}

void UTaskAnimParamLogic::WriteCSV(float time) {

	if (!recorded) {

		UE_LOG(LogAvatarCharacter, Error, TEXT("Time: %f"), time);

		FMyDataTable Row;
		FVector ToBeChangeVector = CuttingBreadCurves.RH_Loc_Interpolation->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		RH_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadCurves.RH_Rot_Interpolation->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		RH_Rot_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadCurves.LH_Loc_Interpolation->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		LH_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadCurves.LH_Rot_Interpolation->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		LH_Rot_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadCurves.Spine_01_Rot_Interpolation->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		S1_Rot_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		if (time >= 10) {

			recorded = true;
			FString ExportingData = RH_Table->GetTableAsCSV();
			FString FileName = "RH_Curve";
			FFileHelper::SaveStringToFile(ExportingData, *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
			UE_LOG(LogTemp, Warning, TEXT("Done! Output: %s"), *(FPaths::ProjectDir() + FString("Datatables/") + FileName));

			ExportingData = RH_Rot_Table->GetTableAsCSV();
			FileName = "RH_Rot_Curve";
			FFileHelper::SaveStringToFile(ExportingData, *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
			UE_LOG(LogTemp, Warning, TEXT("Done! Output: %s"), *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
			
			ExportingData = LH_Rot_Table->GetTableAsCSV();
			FileName = "LH_Rot_Curve";
			FFileHelper::SaveStringToFile(ExportingData, *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
			UE_LOG(LogTemp, Warning, TEXT("Done! Output: %s"), *(FPaths::ProjectDir() + FString("Datatables/") + FileName));

			ExportingData = LH_Table->GetTableAsCSV();
			FileName = "LH_Curve";
			FFileHelper::SaveStringToFile(ExportingData, *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
			UE_LOG(LogTemp, Warning, TEXT("Done! Output: %s"), *(FPaths::ProjectDir() + FString("Datatables/") + FileName));

			ExportingData = S1_Rot_Table->GetTableAsCSV();
			FileName = "Spine01_Rot_Curve";
			FFileHelper::SaveStringToFile(ExportingData, *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
			UE_LOG(LogTemp, Warning, TEXT("Done! Output: %s"), *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
		}
	}
}