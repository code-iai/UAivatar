// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "Components/ActorComponent.h"
#include "IAIAvatarAnimationInstance.h"
#include "Curves/CurveVector.h"

#include "Engine/DataTable.h"
#include "DataTableEditorUtils.h"
#include "UObject/ConstructorHelpers.h"

#include "TaskAnimParamLogic.generated.h"

DECLARE_DELEGATE_OneParam(FRunAnimDelegate, float);
DECLARE_DELEGATE_OneParam(FRunAnimChainDelegate, int);

USTRUCT(BlueprintType)
struct FMyDataTable : public FTableRowBase
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly, Category = "GO")
		float X;

	UPROPERTY(BlueprintReadOnly, Category = "GO")
		float Y;

	UPROPERTY(BlueprintReadOnly, Category = "GO")
		float Z;
};

class DataTableHandler
{
public:

	TArray<TArray<float>> ATable;
	int32 index1;
	int32 index2;

	// Constructors
	DataTableHandler() {};
	DataTableHandler(FString DTPath);

	// Returns total duration time of this table
	float GetDuration();

	// Returns Vector value at specified time
	FVector GetVectorValue(float time);
};

struct CuttingObjData_t
{
public:
	AActor* Object;
	FVector Origin;
	FVector Extent;
	FVector HoldLoc;
	FVector CutLoc;

	float angle;
	float sliceWidth;
};

USTRUCT(Blueprintable)
struct FCurvesSet_t {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* RH_Loc_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* LH_Loc_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* RH_Rot_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* LH_Rot_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* RH_IndexLoc_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* LH_IndexLoc_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveFloat* RH_FingerRots_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveFloat* LH_FingerRots_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* Head_Rot_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* Spine_01_Rot_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		UCurveVector* Jaw_Rot_Interpolation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		float time;
};

struct FAvatarPose_t
{
public:
	FVector RH_Loc;
	FVector LH_Loc;

	FRotator RH_Rot;
	FRotator LH_Rot;

	FVector RH_IndexLoc;
	FVector LH_IndexLoc;

	FFingerRots_t RH_FingerRots;
	FFingerRots_t LH_FingerRots;

	FRotator Spine_01_Rot;
	FRotator Head_Rot;
	FRotator Jaw_Rot;
};

USTRUCT(BlueprintType)
struct FTaskAnimParameters_t
{
	GENERATED_BODY()
public:

	// Help Parameters
	AActor* Object;
	FRunAnimDelegate AnimFunctionDelegate;
	FString ActionContext;
	float animTime;

	// Right Hand
	UCurveVector* RH_Loc_Curve;
	DataTableHandler* RH_Loc_Table;
	FVector RH_Loc_Multiplier;
	FVector RH_Loc_Offset;
	FRotator RH_Loc_Curve_Orientation;

	UCurveVector* RH_Rot_Curve;
	DataTableHandler* RH_Rot_Table;
	FRotator RH_Rot_Multiplier;
	FRotator RH_Rot_Offset;

	bool bUsingRightHand;
	bool bSet_RH_Loc;
	bool bSet_RH_Rot;
	bool bUnSet_RH_Loc;
	bool bUnSet_RH_Rot;

	// Left Hand
	UCurveVector* LH_Loc_Curve;
	DataTableHandler* LH_Loc_Table;
	FVector LH_Loc_Multiplier;
	FVector LH_Loc_Offset;

	UCurveVector* LH_Rot_Curve;
	DataTableHandler* LH_Rot_Table;
	FRotator LH_Rot_Multiplier;
	FRotator LH_Rot_Offset;

	bool bUsingLeftHand;
	bool bSet_LH_Loc;
	bool bSet_LH_Rot;
	bool bUnSet_LH_Loc;
	bool bUnSet_LH_Rot;

	// Right Fingers
	UCurveVector* RH_IndexLoc_Curve;
	FVector RH_IndexLoc_Multiplier;
	FVector RH_IndexLoc_Offset;

	UCurveFloat* RH_FingerRots_Curve;
	FFingerRots_t RH_FingerRots_Offset;
	FFingerRots_t RH_FingerRots_Multiplier;

	bool bSet_RF_Rot;
	bool bSet_RI_Loc;
	bool bUnSet_RI_Loc;
	bool bUnSet_RF_Rot;

	// Left Fingers
	UCurveVector* LH_IndexLoc_Curve;
	FVector LH_IndexLoc_Multiplier;
	FVector LH_IndexLoc_Offset;

	UCurveFloat* LH_FingerRots_Curve;
	FFingerRots_t LH_FingerRots_Offset;
	FFingerRots_t LH_FingerRots_Multiplier;

	bool bSet_LI_Loc;
	bool bSet_LF_Rot;
	bool bUnSet_LI_Loc;
	bool bUnSet_LF_Rot;

	// Spine
	UCurveVector* Spine_01_Rot_Curve;
	DataTableHandler* Spine_01_Rot_Table;
	FRotator Spine_01_Rot_Offset;
	FRotator Spine_01_Rot_Multiplier;

	bool bSet_S01_Rot;
	bool bUnSet_S01_Rot;

	// Head
	UCurveVector* Head_Rot_Curve;
	DataTableHandler* Head_Rot_Table;
	FRotator Head_Rot_Offset;
	FRotator Head_Rot_Multiplier;

	bool bSet_Head_Rot;
	bool bUnSet_Head_Rot;

	// Jaw
	UCurveVector* Jaw_Rot_Curve;
	DataTableHandler* Jaw_Rot_Table;
	FRotator Jaw_Rot_Offset;
	FRotator Jaw_Rot_Multiplier;

	bool bSet_Jaw_Rot;
	bool bUnSet_Jaw_Rot;

	void ClearJointFlags() {

		bSet_RH_Loc = false;
		bSet_RH_Rot = false;
		bUnSet_RH_Loc = false;
		bUnSet_RH_Rot = false;
		bSet_LH_Loc = false;
		bSet_LH_Rot = false;
		bUnSet_LH_Loc = false;
		bUnSet_LH_Rot = false;
		bSet_LI_Loc = false;
		bSet_LF_Rot = false;
		bSet_RI_Loc = false;
		bSet_RF_Rot = false;
		bUnSet_LI_Loc = false;
		bUnSet_LF_Rot = false;
		bUnSet_RI_Loc = false;
		bUnSet_RF_Rot = false;
		bSet_S01_Rot = false;
		bUnSet_S01_Rot = false;
		bSet_Head_Rot = false;
		bUnSet_Head_Rot = false;
		bSet_Jaw_Rot = false;
		bUnSet_Jaw_Rot = false;
	};
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UIAIAVATAR_API UTaskAnimParamLogic : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTaskAnimParamLogic();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t ReadingCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t CuttingBreadCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t CuttingSteakCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t CuttingZucchiniCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t SpooningSoupCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t ForkingCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t PouringCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t PassingPageCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t ReachingCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InterpolationCurves)
		FCurvesSet_t DroppingCurves;

	// This tables are intended for the use of real human motion curves
	UDataTable* RH_Table;
	UDataTable* RH_Rot_Table;
	UDataTable* LH_Table;
	UDataTable* LH_Rot_Table;
	UDataTable* S1_Rot_Table;

	FRunAnimChainDelegate AnimChain;

	float writeTime;
	bool recorded;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	AIAIAvatarCharacter* Avatar;

	UIAIAvatarAnimationInstance* Animation;

	FTaskAnimParameters_t AnimParams;

	FAvatarPose_t OriginalPose;

	CuttingObjData_t Obj2BCut;

	bool bRunAnimation;

	float currentAnimTime;
	float speedFactor;

	int pendingStates;

public:

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ****** Help Functions ****** //

	// Set all needed alphas to run an animation
	void SetJointAlphas();

	// Release all needed alphas to drop a pose
	void UnSetJointAlphas();

	// Set grasping pose parameters
	void SetGraspingObjPose(FAvatarPose_t& Pose);

	// Set holding pose parameters
	void SetHoldingObjPose(FAvatarPose_t& Pose);

	// Attach object to hand
	void AttachObject();

	// Save current pose as original pose
	void SaveOriginalPose();

	// Get current avatar's pose
	FAvatarPose_t GetCurrentAvatarPose();

	// Get current finger rotations
	FFingerRots_t GetCurrentFingersRots(bool isRightHand);

	// Check for item within a list of unique hit results and filter out those out of proper reach
	AActor* CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName);

	// Calculate needed spine rotation to reacha location
	FRotator CalculateSpineRot(FVector LocalEndPoint, float elongation, bool bRotateSpine = false);

	// Calculate head rotation to look to WorlEndPoint
	FRotator CalculateHeadRot(FVector WorldEndPoint);

	// News paper help function for holding locations
	FVector CalculateReachBookLocation(AActor* Book, FName Tag, bool bWorldRelative);

	// Calculate EndPose for passing page
	// vPageLoc and hPageLoc values fron 0 to 1
	void Calculate_PointBook_EndPose_Curves(float vPageLoc, float hPageLoc, FAvatarPose_t& EndPose, FCurvesSet_t& Curves);

	// ****** Cutting Help Functions ****** //

	// Check for cuttable items within a list of unique hit results and filter out those out of proper reach
	TArray<AActor*> CheckForCuttableObjects(TMap<FString, FHitResult> Objects);

	// This will choose the biggest cuttable object 
	AActor* PickOneObject(TArray<AActor*> Cuttables);

	// Check if item is in good position for cutting
	bool isInGoodAlignment();

	// ****** Setting Chain Animations ****** //

	// Reading newspaper
	void StartReadNewspaperAnimChain(AActor* Target, FString Hand = "right");

	// Passing Page
	void StartPassPageAnimChain(AActor* Target, bool bLast = false, FString Hand = "right");

	// Slicing
	void StartSlicingAnimChain();

	// Grasping Object
	void StartGraspingAnimChain(AActor* Target, FString Hand);

	// Placing Object
	void StartPlacingAnimChain(FString targetPlace, FString Hand = "any", FVector Point = FVector(0, 0, 0));

	// Feeding
	void StartFeedingAnimChain(ACharacter* Person);

	// Forking 
	void StartForkAnimChain(AActor* Target);

	// Sliding 
	void StartSlideObjectAnimChain(FString ToSide, AActor* Target);

	// ****** Running Chains ****** //

	// Poiting Book
	void RunReadNewspaperAnimChain(int stage);

	// Passing Page
	void RunPassPageAnimChain(int stage);
	//void RunPassPageAnimChain(int stage, float time);
	// Slicing
	void RunSlicingAnimChain(int stage);

	// Grasping Object
	void RunGraspingAnimChain(int stage);

	// Placing Object
	void RunPlacingAnimChain(int stage);

	// Feeding
	void RunFeedingAnimChain(int stage);

	// Forking
	void RunForkAnimChain(int stage);

	// Sliding
	void RunSlideObjectAnimChain(int stage);

	// ****** Setting Parameters ****** //

	// Setting animation parameters
	void SetAnimParams(FAvatarPose_t StartPose, FAvatarPose_t EndPose, FCurvesSet_t Curves, bool bReleaseAlphas = false);

	// Set parameters for pour animation
	void StartPourAnimation(AActor* Target);

	// Set parameters for Spoon animation
	void StartSpoonAnimation(AActor* Target);

	// Set parameters for looking animation
	void StartLookAnimation(FVector Point);

	// Set parameters for releasing looking animation
	void StartReleaseLookAnimation();

	void StartHandReachAnimation(bool bRightHand, FString Position, FVector Point = FVector(0, 0, 0));

	// ****** Running Animations ****** //

	// Running animation
	void RunAnimation(float time);

	// Running Pour Animation
	void RunPourAnimation(float time);

	// Running fork Animation
	void RunForkAnimation(float time);

	// Running fork Animation
	void RunSpoonAnimation(float time);

	// ****** Processing Task ****** //

	// Spooning
	void CallSpoonAnimation(FString ObjectName);

	// Forking
	void CallForkAnimation(FString ObjectName);

	// Pouring
	void CallPourAnimation(FString ObjectName);

	// Poiting Book
	void CallReadBookAnimChain(FString ObjectName, FString Page);

	// Passing Page
	void CallPassPageAnimChain(FString ObjectName);

	// Closing book
	void CallCloseBookAnimChain(FString ObjectName);

	// Slicing
	void CallSlicingAnimChain(FString ObjectName, float width);

	// Grasping Object
	void CallGraspingAnimChain(FString ObjectName, FString Hand);

	// Placing Object
	void CallPlacingAnimChain(FString ObjectName);

	// Feeding Person
	void CallFeedingAnimChain(FString PersonName);

	// Slide object
	void CallSlideObject(FString ToSide, FString ObjectName);

	void WriteCSV(float time);
};
