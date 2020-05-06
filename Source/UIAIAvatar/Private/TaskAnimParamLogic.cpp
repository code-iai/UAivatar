// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "TaskAnimParamLogic.h"
#include "Engine.h"


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

		UE_LOG(LogAvatarCharacter, Error, TEXT("Tiempo:  %f."), currentAnimTime);

		if(currentAnimTime == 0) 
			SetJointAlphas();

		AnimParams.AnimFunctionDelegate.ExecuteIfBound(currentAnimTime);

		currentAnimTime += DeltaTime * speedFactor;
		if (currentAnimTime > AnimParams.animTime) {
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


AActor* UTaskAnimParamLogic::CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName) {

	FBox ReachArea;
	AActor* Object = NULL;
	FVector ObjLocationInCompSpace = FVector(0, 0, 0);

	// Defining area of proper reach
	ReachArea = FBox::BuildAABB(FVector(45, 0, -20), FVector(25, 20, 5));

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

void UTaskAnimParamLogic::SaveOriginalPose() {

	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	AnimParams.Spine_01_Rot_Original = TempRotOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	AnimParams.Head_Rot_Original = TempRotOut;

	AnimParams.RH_IndexLoc_Original = Avatar->GetMesh()->GetBoneLocation("index_end_r", EBoneSpaces::WorldSpace);
	AnimParams.LH_IndexLoc_Original = Avatar->GetMesh()->GetBoneLocation("index_end_l", EBoneSpaces::WorldSpace);

	AnimParams.RH_FingerRots_Original = GetCurrentFingersRots(true);
	AnimParams.LH_FingerRots_Original = GetCurrentFingersRots(false);

	AnimParams.RH_Loc_Original = Avatar->GetMesh()->GetBoneLocation("hand_r", EBoneSpaces::ComponentSpace);
	AnimParams.RH_Rot_Original = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();

	AnimParams.LH_Loc_Original = Avatar->GetMesh()->GetBoneLocation("hand_l", EBoneSpaces::ComponentSpace);
	AnimParams.LH_Rot_Original = Avatar->GetMesh()->GetBoneQuaternion("hand_l", EBoneSpaces::ComponentSpace).Rotator();
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

// Setting animation chains
void UTaskAnimParamLogic::StartPassPageAnimChain(AActor *Target) {

	pendingStates = 2;
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunPassPageAnimChain);
	AnimParams.ActionContext = "TurnPageBox";
	SaveOriginalPose();

	FVector EndPoint = CalculateReachBookLocation(Target, *AnimParams.ActionContext, true);
	StartReachAnimation("reach_grasp", Target, "right", EndPoint);
}

void UTaskAnimParamLogic::StartCloseBookAnimChain(AActor *Target) {

	pendingStates = 2;
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunPassPageAnimChain);
	AnimParams.ActionContext = "ClosingPageBox";
	SaveOriginalPose();

	FVector EndPoint = CalculateReachBookLocation(Target, *AnimParams.ActionContext, true);
	StartReachAnimation("reach_grasp", Target, "right", EndPoint);
}

void UTaskAnimParamLogic::StartPointBookAnimChain(AActor *Target) {

	pendingStates = 2;
	SaveOriginalPose();
	AnimParams.ActionContext = "TurnPageBox"; // This is a hack to get a location where to point
	AnimChain.BindUObject(this, &UTaskAnimParamLogic::RunPointBookAnimChain);

	FVector EndPoint = CalculateReachBookLocation(Target, *AnimParams.ActionContext, false);
	EndPoint.Y = EndPoint.Y * 1/5;
	EndPoint.X = 0;

	// Relative to world
	EndPoint = AnimParams.Object->GetActorTransform().TransformPosition(EndPoint);

	StartFingerReachAnimation(Target, "right", EndPoint);
}

// Run animation Chains
void UTaskAnimParamLogic::RunPassPageAnimChain(int state) {

	if (pendingStates == 2) {
		StartPassPageAnimation();
	}
	else if (pendingStates == 1) {
		StartReleaseAnimation("drop_grasp","right");
	}

	pendingStates--;
	
}

void UTaskAnimParamLogic::RunPointBookAnimChain(int state) {

	if (pendingStates == 2) {

		FVector EndPoint = CalculateReachBookLocation(AnimParams.Object, *AnimParams.ActionContext, false);
		EndPoint.Y = EndPoint.Y * 4/5;
		EndPoint.X = 0;

		// Relative to world
		EndPoint = AnimParams.Object->GetActorTransform().TransformPosition(EndPoint);

		StartFingerReachAnimation(AnimParams.Object,"right", EndPoint);
		speedFactor = 0.3;
	}
	else if (pendingStates == 1) {
		StartFingerReleaseAnimation("right");
		speedFactor = 1;
	}

	pendingStates--;

}

// Set parameters for task animation
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

void UTaskAnimParamLogic::StartFingerReachAnimation(AActor *Target, FString Hand, FVector Point) {

	// Hand
	FVector LocStartPoint;
	FVector LocEndPoint;
	FVector LocMultiplier;

	FRotator RotStartPoint;
	FRotator RotEndPoint;
	FRotator RotMultiplier;

	// Finger
	FFingerRots_t FingersRotsStartPoint;
	FFingerRots_t FingersRotsEndPoint;
	FFingerRots_t FingersRotsMultiplier;

	// Spine
	FRotator SpineRotStartPoint;
	FRotator SpineRotEndPoint;
	FRotator SpineRotMultiplier;

	// Head 
	FRotator HeadRotStartPoint;
	FRotator HeadRotEndPoint;
	FRotator HeadRotMultiplier;

	FVector LocEndAdjustment = FVector(0, 0, 0);

	AnimParams.ClearJointFlags();

	AnimParams.Object = Target;
	if (Hand.Equals("right"))
		AnimParams.bUsingRightHand = true;

	if (Hand.Equals("left"))
		AnimParams.bUsingLeftHand = true;

	// Start points
	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	SpineRotStartPoint = TempRotOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	HeadRotStartPoint = TempRotOut;

	FingersRotsStartPoint = GetCurrentFingersRots(AnimParams.bUsingRightHand);

	if (AnimParams.bUsingRightHand) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("index_end_r", EBoneSpaces::WorldSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();
	}
	if (AnimParams.bUsingLeftHand) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("index_end_l", EBoneSpaces::WorldSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_l", EBoneSpaces::ComponentSpace).Rotator();
	}

	// End Points
	SpineRotEndPoint = FRotator(0, 25, 0);
	FingersRotsEndPoint = FingersRotsStartPoint;

	if (Target->ActorHasTag("book")) {
		if (AnimParams.bUsingRightHand) {

			RotEndPoint = FRotator(50, -90, 90);
			FingersRotsEndPoint.thumb_01 = FRotator(-35, -20, 70);
			FingersRotsEndPoint.thumb_02 = FRotator(0, -60, 0);
			FingersRotsEndPoint.thumb_03 = FRotator(0, -23, 0);

			FingersRotsEndPoint.index_01 = FRotator(0, -50, -12);
			FingersRotsEndPoint.index_02 = FRotator(-0, -80, 0);
			FingersRotsEndPoint.index_03 = FRotator(-0, -50, 0);

			FingersRotsEndPoint.middle_01 = FRotator(0, -60, -10);
			FingersRotsEndPoint.middle_02 = FRotator(-0, -70, 0);
			FingersRotsEndPoint.middle_03 = FRotator(-0, -60, 0);

			FingersRotsEndPoint.ring_01 = FRotator(-0, -70, -10);
			FingersRotsEndPoint.ring_02 = FRotator(-0, -70, -0);
			FingersRotsEndPoint.ring_03 = FRotator(-0, -60, -0);

			FingersRotsEndPoint.pinky_01 = FRotator(-0, -90,-25);
			FingersRotsEndPoint.pinky_02 = FRotator(-0, -60, -0);
			FingersRotsEndPoint.pinky_03 = FRotator(-0, -60, -0);

		}
	}

	if (Point != FVector(0, 0, 0)) {
		LocEndPoint = Point;
	}
	else {
		LocEndPoint = Target->GetActorLocation();
	}

	HeadRotEndPoint = Avatar->LookingRotationTo(LocEndPoint);

	// Apply Adjusments
	LocEndPoint += LocEndAdjustment;

	// Multiplier
	LocMultiplier = LocEndPoint - LocStartPoint;
	RotMultiplier = RotEndPoint - RotStartPoint;
	SpineRotMultiplier = SpineRotEndPoint - SpineRotStartPoint;
	HeadRotMultiplier = HeadRotEndPoint - HeadRotStartPoint;
	FingersRotsMultiplier = FingersRotsEndPoint - FingersRotsStartPoint;

	// Update AnimParams
	if (AnimParams.bUsingRightHand) {

		AnimParams.RH_IndexLoc_Offset = LocStartPoint;
		AnimParams.RH_IndexLoc_Multiplier = LocMultiplier;
		AnimParams.RH_IndexLoc_Curve = ReachingAnimLocCurve_Hand;

		AnimParams.RH_Rot_Offset = RotStartPoint;
		AnimParams.RH_Rot_Multiplier = RotMultiplier;
		AnimParams.RH_Rot_Curve = ReachingAnimRotCurve_Hand;

		AnimParams.RH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.RH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.RH_FingerRots_Curve = ReachingAnimRotCurve_Fingers;
	}
	if (AnimParams.bUsingLeftHand) {

		AnimParams.LH_IndexLoc_Offset = LocStartPoint;
		AnimParams.LH_IndexLoc_Multiplier = LocMultiplier;
		AnimParams.LH_IndexLoc_Curve = ReachingAnimLocCurve_Hand;

		AnimParams.LH_Rot_Offset = RotStartPoint;
		AnimParams.LH_Rot_Multiplier = RotMultiplier;
		AnimParams.LH_Rot_Curve = ReachingAnimRotCurve_Hand;

		AnimParams.LH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.LH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.LH_FingerRots_Curve = ReachingAnimRotCurve_Fingers;
	}

	AnimParams.Spine_01_Rot_Offset = SpineRotStartPoint;
	AnimParams.Spine_01_Rot_Multiplier = SpineRotMultiplier;
	AnimParams.Spine_01_Rot_Curve = ReachingAnimRotCurve_Spine01;

	AnimParams.Head_Rot_Offset = HeadRotStartPoint;
	AnimParams.Head_Rot_Multiplier = HeadRotMultiplier;
	AnimParams.Head_Rot_Curve = ReachingAnimRotCurve_Head;

	// Enable only the alphas of those joints that will change during animation	
	if (AnimParams.bUsingRightHand) {
		AnimParams.bSet_RI_Loc = true;
		AnimParams.bSet_RH_Rot = true;
		AnimParams.bSet_RF_Rot = true;
	}
	if (AnimParams.bUsingLeftHand) {
		AnimParams.bSet_LI_Loc = true;
		AnimParams.bSet_LH_Rot = true;
		AnimParams.bSet_LF_Rot = true;
	}

	AnimParams.bSet_S01_Rot = true;
	AnimParams.bSet_Head_Rot = true;

	AnimParams.animTime = 1.25;
	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunReachAnimation);
	bRunAnimation = true;
}

void UTaskAnimParamLogic::StartReachAnimation(FString Type, AActor *Target, FString Hand, FVector Point) {

	// Hand
	FVector LocStartPoint;
	FVector LocEndPoint;
	FVector LocMultiplier;

	FRotator RotStartPoint;
	FRotator RotEndPoint;
	FRotator RotMultiplier;
	
	// Finger
	FFingerRots_t FingersRotsStartPoint;
	FFingerRots_t FingersRotsEndPoint;
	FFingerRots_t FingersRotsMultiplier;

	// Spine
	FRotator SpineRotStartPoint;
	FRotator SpineRotEndPoint;
	FRotator SpineRotMultiplier;

	// Head 
	FRotator HeadRotStartPoint;
	FRotator HeadRotEndPoint;
	FRotator HeadRotMultiplier;

	FVector LocEndAdjustment = FVector(-7, -7, 15);
	
	AnimParams.ClearJointFlags();

	AnimParams.Object = Target;
	if (Hand.Equals("right"))
		AnimParams.bUsingRightHand = true;

	if (Hand.Equals("left"))
		AnimParams.bUsingLeftHand = true;

	// Start points
	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	SpineRotStartPoint = TempRotOut;
	
	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	HeadRotStartPoint = TempRotOut;

	FingersRotsStartPoint = GetCurrentFingersRots(AnimParams.bUsingRightHand);

	if (AnimParams.bUsingRightHand) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("hand_r", EBoneSpaces::ComponentSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();
	}
	if (AnimParams.bUsingLeftHand) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("hand_l", EBoneSpaces::ComponentSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_l", EBoneSpaces::ComponentSpace).Rotator();
	}

	// End Points
	SpineRotEndPoint = FRotator(0, 25, 0);
	FingersRotsEndPoint = FingersRotsStartPoint;

	if (Target->ActorHasTag("book")) {
		if (AnimParams.bUsingRightHand) {

			LocEndAdjustment = FVector(-5, 0, 5);
			RotEndPoint = FRotator(60, 0, 120);
			FingersRotsEndPoint.thumb_01 = FRotator(10, -50, 140);
			FingersRotsEndPoint.thumb_02 = FRotator(0, -0, 0);
			FingersRotsEndPoint.thumb_03 = FRotator(0, -0, 0);

			FingersRotsEndPoint.index_01 = FRotator(-10,-65,0);
			FingersRotsEndPoint.index_02 = FRotator(-0, -45, 0);
			FingersRotsEndPoint.index_03 = FRotator(-0, -45, 0);
			
			FingersRotsEndPoint.middle_01 = FRotator(-15, -65, 0);
			FingersRotsEndPoint.middle_02 = FRotator(-0, -45, 0);
			FingersRotsEndPoint.middle_03 = FRotator(-0, -45, 0);

			FingersRotsEndPoint.ring_01 = FRotator(-10, -45, 0);
			FingersRotsEndPoint.pinky_01 = FRotator(-10, -45, 0);

		}
	}

	if (Point != FVector(0, 0, 0)) {
		LocEndPoint = Point;
	}
	else {
		LocEndPoint = Target->GetActorLocation();
	}
	
	// Head end Rotation
	HeadRotEndPoint = Avatar->LookingRotationTo(LocEndPoint);

	// Relative to Avatar
	LocEndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(LocEndPoint);

	// Apply Adjusments
	LocEndPoint += LocEndAdjustment;

	// Multiplier
	LocMultiplier = LocEndPoint - LocStartPoint;
	RotMultiplier = RotEndPoint - RotStartPoint;
	SpineRotMultiplier = SpineRotEndPoint - SpineRotStartPoint;
	HeadRotMultiplier = HeadRotEndPoint - HeadRotStartPoint;
	FingersRotsMultiplier = FingersRotsEndPoint - FingersRotsStartPoint;

	// Update AnimParams
	if (AnimParams.bUsingRightHand) {

		AnimParams.RH_Loc_Offset = LocStartPoint;
		AnimParams.RH_Loc_Multiplier = LocMultiplier;
		AnimParams.RH_Loc_Curve = ReachingAnimLocCurve_Hand;

		AnimParams.RH_Rot_Offset = RotStartPoint;
		AnimParams.RH_Rot_Multiplier = RotMultiplier;
		AnimParams.RH_Rot_Curve = ReachingAnimRotCurve_Hand;

		AnimParams.RH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.RH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.RH_FingerRots_Curve = ReachingAnimRotCurve_Fingers;
	}
	if (AnimParams.bUsingLeftHand) {

		AnimParams.LH_Loc_Offset = LocStartPoint;
		AnimParams.LH_Loc_Multiplier = LocMultiplier;
		AnimParams.LH_Loc_Curve = ReachingAnimLocCurve_Hand;

		AnimParams.LH_Rot_Offset = RotStartPoint;
		AnimParams.LH_Rot_Multiplier = RotMultiplier;
		AnimParams.LH_Rot_Curve = ReachingAnimRotCurve_Hand;

		AnimParams.LH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.LH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.LH_FingerRots_Curve = ReachingAnimRotCurve_Fingers;
	}

	AnimParams.Spine_01_Rot_Offset = SpineRotStartPoint;
	AnimParams.Spine_01_Rot_Multiplier = SpineRotMultiplier;
	AnimParams.Spine_01_Rot_Curve = ReachingAnimRotCurve_Spine01;

	AnimParams.Head_Rot_Offset = HeadRotStartPoint;
	AnimParams.Head_Rot_Multiplier = HeadRotMultiplier;
	AnimParams.Head_Rot_Curve = ReachingAnimRotCurve_Head;

	// Enable only the alphas of those joints that will change during animation
	if (Type.Equals("reach_grasp") || Type.Equals("reach_grasp_take")) {
		if (AnimParams.bUsingRightHand) {
			AnimParams.bSet_RH_Loc = true;
			AnimParams.bSet_RH_Rot = true;
			AnimParams.bSet_RF_Rot = true;
		}
		if (AnimParams.bUsingLeftHand) {
			AnimParams.bSet_LH_Loc = true;
			AnimParams.bSet_LH_Rot = true;
			AnimParams.bSet_LF_Rot = true;
		}
	}
	else if (Type.Equals("reach")){
		if (AnimParams.bUsingRightHand) {
			AnimParams.bSet_RH_Loc = true;
			AnimParams.bSet_RH_Rot = true;
		}
		if (AnimParams.bUsingLeftHand) {
			AnimParams.bSet_LH_Loc = true;
			AnimParams.bSet_LH_Rot = true;
		}
	}

	AnimParams.bSet_S01_Rot = true;
	AnimParams.bSet_Head_Rot = true;

	AnimParams.animTime = 1.25;
	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunReachAnimation);
	bRunAnimation = true;
}

void UTaskAnimParamLogic::StartPassPageAnimation() {

	// Hand
	FVector LocStartPoint;
	FVector LocEndPoint;
	FVector LocMultiplier;

	FRotator RotStartPoint;
	FRotator RotEndPoint;
	FRotator RotMultiplier;

	// Fingers
	FFingerRots_t FingersRotsStartPoint;
	FFingerRots_t FingersRotsEndPoint;
	FFingerRots_t FingersRotsMultiplier;

	// Spine
	FRotator SpineRotStartPoint;
	FRotator SpineRotEndPoint;
	FRotator SpineRotMultiplier;

	// Head 
	FRotator HeadRotStartPoint;
	FRotator HeadRotEndPoint;
	FRotator HeadRotMultiplier;

	// StarPoints
	if (AnimParams.bUsingRightHand) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("hand_r", EBoneSpaces::ComponentSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();
	}
	else if (AnimParams.bUsingLeftHand) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("hand_l", EBoneSpaces::ComponentSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_l", EBoneSpaces::ComponentSpace).Rotator();
	}

	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	SpineRotStartPoint = TempRotOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	HeadRotStartPoint = TempRotOut;

	FingersRotsStartPoint = GetCurrentFingersRots(AnimParams.bUsingRightHand);

	// EndPoints
	SpineRotEndPoint = FRotator(-15, 40, 40); // This start point is specific for passing page chain
	FingersRotsEndPoint = FingersRotsStartPoint;

	// Pass Page Box Location 
	FVector EndPoint = CalculateReachBookLocation(AnimParams.Object, *AnimParams.ActionContext, false);

	// Get oposite side of book
	EndPoint.Y *= -1;

	// Get looking point
	FVector LookingPoint = EndPoint;
	LookingPoint = AnimParams.Object->GetActorTransform().TransformPosition(LookingPoint);
	HeadRotEndPoint = Avatar->LookingRotationTo(LookingPoint);

	// Modify end point for multiplier 
	EndPoint.Y += 10;
	EndPoint.Z += EndPoint.Y;
	
	// Relative to world
	EndPoint = AnimParams.Object->GetActorTransform().TransformPosition(EndPoint);

	// Relative to Avatar
	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);

	LocEndPoint = EndPoint + FVector(0, 0, 7);

						 // Hand Rotation trajectory
	                     // Pitch Yaw  Roll
						 //  Y     Z     X
	                     // 60      0   120 
						 //  0    -90    90
	                     // 60   -150   120 
	RotEndPoint = FRotator(0, -150, 90);

	// Multipliers
	LocMultiplier = LocEndPoint - LocStartPoint;
	RotMultiplier = RotEndPoint - RotStartPoint;
	SpineRotMultiplier = SpineRotEndPoint - SpineRotStartPoint;
	HeadRotMultiplier = HeadRotEndPoint - HeadRotStartPoint;
	FingersRotsMultiplier = FingersRotsEndPoint - FingersRotsStartPoint;

	// Update AnimParams
	if (AnimParams.bUsingRightHand) {

		AnimParams.RH_Loc_Offset = LocStartPoint;
		AnimParams.RH_Loc_Multiplier = LocMultiplier;
		AnimParams.RH_Loc_Curve = PassingPageAnimLocCurve_Hand;

		AnimParams.RH_Rot_Offset = RotStartPoint;
		AnimParams.RH_Rot_Multiplier = RotMultiplier;
		AnimParams.RH_Rot_Curve = PassingPageAnimRotCurve_Hand;

		AnimParams.RH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.RH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.RH_FingerRots_Curve = PassingPageAnimRotCurve_Fingers;
	}
	if (AnimParams.bUsingLeftHand) {

		AnimParams.LH_Loc_Offset = LocStartPoint;
		AnimParams.LH_Loc_Multiplier = LocMultiplier;
		AnimParams.LH_Loc_Curve = PassingPageAnimLocCurve_Hand;

		AnimParams.LH_Rot_Offset = RotStartPoint;
		AnimParams.LH_Rot_Multiplier = RotMultiplier;
		AnimParams.LH_Rot_Curve = PassingPageAnimRotCurve_Hand;

		AnimParams.LH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.LH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.LH_FingerRots_Curve = PassingPageAnimRotCurve_Fingers;
	}

	AnimParams.Spine_01_Rot_Offset = SpineRotStartPoint;
	AnimParams.Spine_01_Rot_Multiplier = SpineRotMultiplier;
	AnimParams.Spine_01_Rot_Curve = PassingPageAnimRotCurve_Spine01;

	AnimParams.Head_Rot_Offset = HeadRotStartPoint;
	AnimParams.Head_Rot_Multiplier = HeadRotMultiplier;
	AnimParams.Head_Rot_Curve = ReachingAnimRotCurve_Head;

	if (AnimParams.bUsingRightHand) {
		AnimParams.bSet_RF_Rot = false;
	}
	if (AnimParams.bUsingLeftHand) {
		AnimParams.bSet_LF_Rot = false;
	}

	AnimParams.animTime = 2.5;
	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunReachAnimation);
	bRunAnimation = true;
}

void UTaskAnimParamLogic::StartFingerReleaseAnimation(FString Hand) {

	// Hand
	FVector LocStartPoint;
	FVector LocEndPoint;
	FVector LocMultiplier;

	FRotator RotStartPoint;
	FRotator RotEndPoint;
	FRotator RotMultiplier;

	// Fingers
	FFingerRots_t FingersRotsStartPoint;
	FFingerRots_t FingersRotsEndPoint;
	FFingerRots_t FingersRotsMultiplier;

	// Spine
	FRotator SpineRotStartPoint;
	FRotator SpineRotEndPoint;
	FRotator SpineRotMultiplier;

	// Head 
	FRotator HeadRotStartPoint;
	FRotator HeadRotEndPoint;
	FRotator HeadRotMultiplier;

	// StartPoints & EndPoints
	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	SpineRotStartPoint = TempRotOut;
	SpineRotEndPoint = AnimParams.Spine_01_Rot_Original;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	HeadRotStartPoint = TempRotOut;
	HeadRotEndPoint = AnimParams.Head_Rot_Original;

	if (Hand.Equals("right")) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("index_end_r", EBoneSpaces::WorldSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();
		FingersRotsStartPoint = GetCurrentFingersRots(true);

		LocEndPoint = AnimParams.RH_IndexLoc_Original;
		RotEndPoint = AnimParams.RH_Rot_Original;
		FingersRotsEndPoint = AnimParams.RH_FingerRots_Original;
	}
	else if (Hand.Equals("left")) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("index_end_l", EBoneSpaces::WorldSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_l", EBoneSpaces::ComponentSpace).Rotator();
		FingersRotsStartPoint = GetCurrentFingersRots(false);

		LocEndPoint = AnimParams.LH_IndexLoc_Original;
		RotEndPoint = AnimParams.LH_Rot_Original;
		FingersRotsEndPoint = AnimParams.LH_FingerRots_Original;
	}

	// Multipliers
	LocMultiplier = LocEndPoint - LocStartPoint;
	RotMultiplier = RotEndPoint - RotStartPoint;
	SpineRotMultiplier = SpineRotEndPoint - SpineRotStartPoint;
	HeadRotMultiplier = HeadRotEndPoint - HeadRotStartPoint;
	FingersRotsMultiplier = FingersRotsEndPoint - FingersRotsStartPoint;

	// Update AnimParams
	if (Hand.Equals("right")) {

		AnimParams.RH_IndexLoc_Offset = LocStartPoint;
		AnimParams.RH_IndexLoc_Multiplier = LocMultiplier;
		AnimParams.RH_IndexLoc_Curve = DroppingAnimLocCurve_Hand;

		AnimParams.RH_Rot_Offset = RotStartPoint;
		AnimParams.RH_Rot_Multiplier = RotMultiplier;
		AnimParams.RH_Rot_Curve = DroppingAnimRotCurve_Hand;

		AnimParams.RH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.RH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.RH_FingerRots_Curve = DroppingAnimRotCurve_Fingers;
	}
	if (Hand.Equals("left")) {

		AnimParams.LH_IndexLoc_Offset = LocStartPoint;
		AnimParams.LH_IndexLoc_Multiplier = LocMultiplier;
		AnimParams.LH_IndexLoc_Curve = DroppingAnimLocCurve_Hand;

		AnimParams.LH_Rot_Offset = RotStartPoint;
		AnimParams.LH_Rot_Multiplier = RotMultiplier;
		AnimParams.LH_Rot_Curve = DroppingAnimRotCurve_Hand;

		AnimParams.LH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.LH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.LH_FingerRots_Curve = DroppingAnimRotCurve_Fingers;
	}

	AnimParams.Spine_01_Rot_Offset = SpineRotStartPoint;
	AnimParams.Spine_01_Rot_Multiplier = SpineRotMultiplier;
	AnimParams.Spine_01_Rot_Curve = DroppingAnimRotCurve_Spine01;

	AnimParams.Head_Rot_Offset = HeadRotStartPoint;
	AnimParams.Head_Rot_Multiplier = HeadRotMultiplier;
	AnimParams.Head_Rot_Curve = DroppingAnimRotCurve_Head;

	// Enable only the alphas of those joints that will change during animation.
	// And unset only those that are going to be released at the end.
	
	if (Hand.Equals("right")) {
		AnimParams.bSet_RI_Loc = true;
		AnimParams.bSet_RH_Rot = true;
		AnimParams.bSet_RF_Rot = true;
		AnimParams.bUnSet_RI_Loc = true;
		AnimParams.bUnSet_RH_Rot = true;
		AnimParams.bUnSet_RF_Rot = true;
	}
	else if (Hand.Equals("left")) {
		AnimParams.bSet_LI_Loc = true;
		AnimParams.bSet_LH_Rot = true;
		AnimParams.bSet_LF_Rot = true;
		AnimParams.bUnSet_LI_Loc = true;
		AnimParams.bUnSet_LH_Rot = true;
		AnimParams.bUnSet_LF_Rot = true;
	}
	
	AnimParams.bSet_S01_Rot = true;
	AnimParams.bSet_Head_Rot = true;
	AnimParams.bUnSet_S01_Rot = true;
	AnimParams.bUnSet_Head_Rot = true;

	AnimParams.animTime = 1.25;
	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunReachAnimation);
	bRunAnimation = true;
}

void UTaskAnimParamLogic::StartReleaseAnimation(FString Type, FString Hand) {

	// Hand
	FVector LocStartPoint;
	FVector LocEndPoint;
	FVector LocMultiplier;

	FRotator RotStartPoint;
	FRotator RotEndPoint;
	FRotator RotMultiplier;

	// Fingers
	FFingerRots_t FingersRotsStartPoint;
	FFingerRots_t FingersRotsEndPoint;
	FFingerRots_t FingersRotsMultiplier;

	// Spine
	FRotator SpineRotStartPoint;
	FRotator SpineRotEndPoint;
	FRotator SpineRotMultiplier;

	// Head 
	FRotator HeadRotStartPoint;
	FRotator HeadRotEndPoint;
	FRotator HeadRotMultiplier;

	// StartPoints & EndPoints
	FRotator TempRotIn, TempRotOut;
	FVector TempVecIn, TempVecOut;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("spine_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("Pelvis", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	SpineRotStartPoint = TempRotOut;
	SpineRotEndPoint = AnimParams.Spine_01_Rot_Original;

	TempRotIn = Avatar->GetMesh()->GetBoneQuaternion("neck_01", EBoneSpaces::WorldSpace).Rotator();
	Avatar->GetMesh()->TransformToBoneSpace("spine_03", TempVecIn, TempRotIn, TempVecOut, TempRotOut);
	HeadRotStartPoint = TempRotOut;
	HeadRotEndPoint = AnimParams.Head_Rot_Original;

	if (Hand.Equals("right")) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("hand_r", EBoneSpaces::ComponentSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_r", EBoneSpaces::ComponentSpace).Rotator();
		FingersRotsStartPoint = GetCurrentFingersRots(true);

		LocEndPoint = AnimParams.RH_Loc_Original;
		RotEndPoint = AnimParams.RH_Rot_Original;
		FingersRotsEndPoint = AnimParams.RH_FingerRots_Original;
	}
	else if (Hand.Equals("left")) {
		LocStartPoint = Avatar->GetMesh()->GetBoneLocation("hand_l", EBoneSpaces::ComponentSpace);
		RotStartPoint = Avatar->GetMesh()->GetBoneQuaternion("hand_l", EBoneSpaces::ComponentSpace).Rotator();
		FingersRotsStartPoint = GetCurrentFingersRots(false);

		LocEndPoint = AnimParams.LH_Loc_Original;
		RotEndPoint = AnimParams.LH_Rot_Original;
		FingersRotsEndPoint = AnimParams.LH_FingerRots_Original;
	}

	// Multipliers
	LocMultiplier = LocEndPoint - LocStartPoint;
	RotMultiplier = RotEndPoint - RotStartPoint;
	SpineRotMultiplier = SpineRotEndPoint - SpineRotStartPoint;
	FingersRotsMultiplier = FingersRotsEndPoint - FingersRotsStartPoint;
	HeadRotMultiplier = HeadRotEndPoint - HeadRotStartPoint;

	// Update AnimParams
	if (Hand.Equals("right")) {

		AnimParams.RH_Loc_Offset = LocStartPoint;
		AnimParams.RH_Loc_Multiplier = LocMultiplier;
		AnimParams.RH_Loc_Curve = DroppingAnimLocCurve_Hand;

		AnimParams.RH_Rot_Offset = RotStartPoint;
		AnimParams.RH_Rot_Multiplier = RotMultiplier;
		AnimParams.RH_Rot_Curve = DroppingAnimRotCurve_Hand;

		AnimParams.RH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.RH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.RH_FingerRots_Curve = DroppingAnimRotCurve_Fingers;
	}
	if (Hand.Equals("left")) {

		AnimParams.LH_Loc_Offset = LocStartPoint;
		AnimParams.LH_Loc_Multiplier = LocMultiplier;
		AnimParams.LH_Loc_Curve = DroppingAnimLocCurve_Hand;

		AnimParams.LH_Rot_Offset = RotStartPoint;
		AnimParams.LH_Rot_Multiplier = RotMultiplier;
		AnimParams.LH_Rot_Curve = DroppingAnimRotCurve_Hand;

		AnimParams.LH_FingerRots_Offset = FingersRotsStartPoint;
		AnimParams.LH_FingerRots_Multiplier = FingersRotsMultiplier;
		AnimParams.LH_FingerRots_Curve = DroppingAnimRotCurve_Fingers;
	}

	AnimParams.Spine_01_Rot_Offset = SpineRotStartPoint;
	AnimParams.Spine_01_Rot_Multiplier = SpineRotMultiplier;
	AnimParams.Spine_01_Rot_Curve = DroppingAnimRotCurve_Spine01;

	AnimParams.Head_Rot_Offset = HeadRotStartPoint;
	AnimParams.Head_Rot_Multiplier = HeadRotMultiplier;
	AnimParams.Head_Rot_Curve = DroppingAnimRotCurve_Head;

	// Enable only the alphas of those joints that will change during animation.
	// And unset only those that are going to be released at the end.
	if (Type.Equals("drop_grasp") || Type.Equals("drop_grasp_take")) {
		if (Hand.Equals("right")) {
			AnimParams.bSet_RH_Loc = true;
			AnimParams.bSet_RH_Rot = true;
			AnimParams.bSet_RF_Rot = true;
			AnimParams.bUnSet_RH_Loc = true;
			AnimParams.bUnSet_RH_Rot = true;
			AnimParams.bUnSet_RF_Rot = true;
		} else if (Hand.Equals("left")) {
			AnimParams.bSet_LH_Loc = true;
			AnimParams.bSet_LH_Rot = true;
			AnimParams.bSet_LF_Rot = true;
			AnimParams.bUnSet_LH_Loc = true;
			AnimParams.bUnSet_LH_Rot = true;
			AnimParams.bUnSet_LF_Rot = true;
		}
	}
	else if (Type.Equals("drop")) {
		if (Hand.Equals("right")) {
			AnimParams.bSet_RH_Loc = true;
			AnimParams.bSet_RH_Rot = true;
			AnimParams.bUnSet_RH_Loc = true;
			AnimParams.bUnSet_RH_Rot = true;
		} else if (Hand.Equals("left")) {
			AnimParams.bSet_LH_Loc = true;
			AnimParams.bSet_LH_Rot = true;
			AnimParams.bUnSet_LH_Loc = true;
			AnimParams.bUnSet_LH_Rot = true;
		}
	}

	AnimParams.bSet_S01_Rot = true;
	AnimParams.bSet_Head_Rot = true;
	AnimParams.bUnSet_S01_Rot = true;
	AnimParams.bUnSet_Head_Rot = true;

	AnimParams.animTime = 1.25;
	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunReachAnimation);
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
		AnimParams.RH_Loc_Curve = CuttingBreadAnimCurve;
		AnimParams.RH_Rot_Curve = CuttingBreadAnimRotCurve;
		AnimParams.LH_Loc_Curve = CuttingBreadAnimCurve_LH;
		AnimParams.LH_Rot_Curve = CuttingBreadAnimRotCurve_LH;
		AnimParams.Spine_01_Rot_Curve = CuttingBreadAnimRotCurve_Spine01;

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

	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier;
	FVector EndAdjustment = FVector(-7, -7, 15);

	AnimParams.Spine_01_Rot_Multiplier = FRotator(0, 15, 0);
	AnimParams.Spine_01_Rot_Curve = ForkingAnimSpineRotCurve;

	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Target->GetActorLocation());
	EndPoint += EndAdjustment;

	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = (EndPoint - StartPoint);

	//
	AnimParams.RH_Loc_Offset = StartPoint;
	AnimParams.RH_Loc_Multiplier = Multiplier;

	// Skill
	AnimParams.RH_Loc_Curve = ForkingAnimCurve;
	AnimParams.RH_Rot_Curve = ForkingAnimRotCurve;

	AnimParams.animTime = 4.5;

	AnimParams.bSet_LH_Loc = false;
	AnimParams.bSet_LH_Rot = false;
	// These 2 are active from grasp and hold
	AnimParams.bSet_RH_Loc = false;
	AnimParams.bSet_RH_Rot = false;
	AnimParams.bSet_RF_Rot = false;
	AnimParams.bSet_LF_Rot = false;
	AnimParams.bSet_S01_Rot = true;

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

	AnimParams.AnimFunctionDelegate.BindUObject(this, &UTaskAnimParamLogic::RunPourAnimation);
	bRunAnimation = true;
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
	Hand_r_Location *= AnimParams.RH_Loc_Multiplier;
	Hand_r_Location += AnimParams.RH_Loc_Offset;

	Animation->RightHandRotation = Hand_r_Rotation;
	Animation->RightHandIKTargetPosition = Hand_r_Location;
	Temp = AnimParams.Spine_01_Rot_Curve->GetVectorValue(time);
	Animation->Spine1Rotation = FRotator(Temp.Y, Temp.Z, Temp.X);

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

// Run fork animation
void UTaskAnimParamLogic::RunReachAnimation(float time) {

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
			ObjectData_t CurrentCutable;
			if (Object->GetRootComponent()->ComponentHasTag("Cuttable")) {
				CurrentCutable.Object = Object;
				if (isInGoodAlignment(CurrentCutable)) {
					StartCutAnimation(CurrentCutable);
				}
			}
		}
		else if (task.Equals("pour")) {
			StartPourAnimation(Object);
		}
		else if (task.Equals("fork")) {
			StartForkAnimation(Object);
		}
		else if (task.Equals("spoon")) {
			StartSpoonAnimation(Object);
		}
		else if (task.Equals("pass")) {
			if (Object->ActorHasTag("book")) {
				StartPassPageAnimChain(Object);
			}
		}
		else if (task.Equals("close")) {
			if (Object->ActorHasTag("book")) {
				StartCloseBookAnimChain(Object);
			}
		}
		else if (task.Equals("point")) {
			if (Object->ActorHasTag("book")) {
				StartPointBookAnimChain(Object);
			}
		}
	} else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
	}
}

void UTaskAnimParamLogic::ProcessTask_P_ObjectName(FString task, FString ObjectName) {
	
	ApplyTaskOnActor(task, CheckForObject(Avatar->ListObjects(), ObjectName));
}


void UTaskAnimParamLogic::WriteCSV(float time) {

	if (!recorded) {

		UE_LOG(LogAvatarCharacter, Error, TEXT("Time: %f"), time);

		FMyDataTable Row;
		FVector ToBeChangeVector = CuttingBreadAnimCurve->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		RH_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadAnimRotCurve->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		RH_Rot_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadAnimCurve_LH->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		LH_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadAnimRotCurve_LH->GetVectorValue(time);
		Row.X = ToBeChangeVector.X;
		Row.Y = ToBeChangeVector.Y;
		Row.Z = ToBeChangeVector.Z;

		LH_Rot_Table->AddRow(FName(*FString::SanitizeFloat(time)), Row);

		ToBeChangeVector = CuttingBreadAnimRotCurve_Spine01->GetVectorValue(time);
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