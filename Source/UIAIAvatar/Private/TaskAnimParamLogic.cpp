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

}


// Called every frame
void UTaskAnimParamLogic::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
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

AActor* UTaskAnimParamLogic::CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName) {

	FBox ReachArea;
	AActor* Object=  NULL;
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
FTaskAnimParameters_t UTaskAnimParamLogic::calculateCutAnimParameters(CuttableObjectData_t &ItemData) {

	FTaskAnimParameters_t AnimParams;
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
		AnimParams.RH_Curve = CuttingBreadAnimCurve;
		AnimParams.RH_Rotation_Curve = CuttingBreadAnimRotCurve;
		AnimParams.animTime = 8;
		AnimParams.Task = "CutBread";
		sliceWidth = 2;

		// Right Hand
		HoldAdjusment = FVector(-5, 0, 2);
		StartAdjustment = FVector(-18, 0, 5);
		EndAdjustment = FVector(-18, 0, 5.3);
		Multiplier = FVector(1, 4, 1);

		// Left Hand
		HoldRotation = FRotator(0, 90, -80);
	}
	else if (ItemData.Object->ActorHasTag("Steak")) {
		
		// Skill
		AnimParams.RH_Curve = CuttingSteakAnimCurve;
		AnimParams.RH_Rotation_Curve = CuttingSteakAnimRotCurve;
		AnimParams.animTime = 2;
		AnimParams.Task = "CutBread";
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
		AnimParams.RH_Curve = CuttingZucchiniAnimCurve;
		AnimParams.RH_Rotation_Curve = CuttingZucchiniAnimRotCurve;
		AnimParams.animTime = 1.6;
		AnimParams.Task = "CutBread";
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

	// Calculate Start Point
	StartPoint.X = 0;
	StartPoint.Y = -1 * halfLength + sliceWidth;
	StartPoint.Z = ItemData.Extent.Z;

	// Calculate End Point
	EndPoint.X = StartPoint.X;
	EndPoint.Y = StartPoint.Y;
	EndPoint.Z = -1 * ItemData.Extent.Z;

	// Calculate hold point
	HoldPoint.X = 0;
	HoldPoint.Y = -1 * halfLength + holdingDistance;
	HoldPoint.Z = ItemData.Extent.Z;

	// Adjust coordinates. The IK used is controling the Avatar's wrist that shouldn't be exactly on
	// the working points but maybe a bit above and behind.
	StartPoint += StartAdjustment;
	EndPoint += EndAdjustment;
	HoldPoint += HoldAdjusment;

	// Put back to original order
	if (!orderWidthLenght) {

		tmp = StartPoint.X;
		StartPoint.X = StartPoint.Y;
		StartPoint.Y = tmp;

		tmp = EndPoint.X;
		EndPoint.X = EndPoint.Y;
		EndPoint.Y = tmp;

		tmp = HoldPoint.X;
		HoldPoint.X = HoldPoint.Y;
		HoldPoint.Y = tmp;
	}

	// We are always cutting on the right extreme of the object according to Avatar's point of view.
	if (!(ItemData.angle > 354 || ItemData.angle < -354 || (ItemData.angle < 6 && ItemData.angle > -6))) {
		StartPoint = StartPoint.RotateAngleAxis(180, FVector(0, 0, 1));
		EndPoint = EndPoint.RotateAngleAxis(180, FVector(0, 0, 1));
		HoldPoint = HoldPoint.RotateAngleAxis(180, FVector(0, 0, 1));
		ItemData.angle -= 180;
	}

	// Calculate Orientation
	LocalRot = FRotator(0, ItemData.angle, 0);

	// Convert coordinates relative to world
	StartPoint = ItemData.Object->GetActorTransform().TransformPosition(StartPoint);
	EndPoint   = ItemData.Object->GetActorTransform().TransformPosition(EndPoint);
	HoldPoint  = ItemData.Object->GetActorTransform().TransformPosition(HoldPoint);

	// Convert points relative to Avatar. This is best to adjust points
	StartPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(StartPoint);
	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(EndPoint);
	HoldPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(HoldPoint);

	// Motion Depth
	Multiplier.Z *= -1 * (StartPoint.Z - EndPoint.Z);

	// Put back in world coordinates
	AnimParams.RH_Curve_Offset = StartPoint;
	AnimParams.RH_Curve_Multiplier = Multiplier;
	AnimParams.LH_Rotation = HoldRotation;
	AnimParams.LH_Location  = HoldPoint;

	AnimParams.RH_Curve_Orientation = LocalRot;

	AnimParams.success = true;

	return AnimParams;
}
#pragma optimize("", on)

FTaskAnimParameters_t UTaskAnimParamLogic::calculatePourAnimParameters(AActor* Target) {

	FTaskAnimParameters_t AnimParams;
	FVector StartPoint;
	FVector EndPoint;
	FVector Multiplier;
	FVector EndAdjustment = FVector(-15, -5, 18);

	EndPoint = Avatar->GetMesh()->GetComponentTransform().InverseTransformPosition(Target->GetActorLocation());
	EndPoint += EndAdjustment;

	StartPoint = FVector(-15, 15, 105);

	// Motion Depth
	Multiplier = (EndPoint - StartPoint);

	//
	AnimParams.RH_Curve_Offset = StartPoint;
	AnimParams.RH_Curve_Multiplier = Multiplier;

	// Skill
	AnimParams.RH_Curve = PouringAnimCurve;
	AnimParams.RH_Rotation_Curve = PouringAnimRotCurve;

	AnimParams.animTime = 9;
	AnimParams.Task = "Pour";

	return AnimParams;
}

// Process a task request
FTaskAnimParameters_t UTaskAnimParamLogic::processTask(FString task) {

	AActor* Object;
	FTaskAnimParameters_t AnimParams;

	if (task.Equals("Cut")) {
		Object = PickOneObject(CheckForCuttableObjects(Avatar->ListObjects()));
		if (Object != nullptr) {
			AnimParams = processTaskOn(task, Object);
		}
		else {
			UE_LOG(LogAvatarCharacter, Error, TEXT("Error: not able to find any cuttable object."));
			AnimParams.success = false;
		}
	}
	else if (task.Equals("Spoon")) {
	}

	return AnimParams;
}

// Process a task request on specific object
FTaskAnimParameters_t UTaskAnimParamLogic::processTaskOn(FString task, AActor* Object) {

	CuttableObjectData_t CurrentCutable;
	FTaskAnimParameters_t AnimParams;

	if (Object != NULL) {
		if (task.Equals("Cut")) {
			if (Object->GetRootComponent()->ComponentHasTag("Cuttable")) {
				CurrentCutable.Object = Object;
				if (isInGoodAlignment(CurrentCutable)) {
					AnimParams = calculateCutAnimParameters(CurrentCutable);
				}
				else {
					AnimParams.success = false;
				}
			}
		}
		else if (task.Equals("PourOver")) {

			AnimParams = calculatePourAnimParameters(Object);
		}
		else {
			AnimParams.success = false;
		}
	}
	else {
		UE_LOG(LogAvatarCharacter, Error, TEXT("Error: no valid object provided."));
		AnimParams.success = false;
	}

	return AnimParams;
}

