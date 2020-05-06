// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "Components/ActorComponent.h"
#include "IAIAvatarAnimationInstance.h"
#include "Curves/CurveVector.h"

#include "Engine/DataTable.h"
#include "DataTableEditorUtils.h"
#include "ConstructorHelpers.h"

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

struct ObjectData_t
{
public:
	AActor* Object;
	float angle;
	FRotator OriginalRotation;
	FVector Origin;
	FVector Extent;
};

USTRUCT(BlueprintType)
struct FTaskAnimParameters_t
{
	GENERATED_BODY()
public:

	// Help Parameters
	AActor *Object;
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

	// Original positions
	FVector RH_Loc_Original;
	FVector LH_Loc_Original;
	FRotator RH_Rot_Original;
	FRotator LH_Rot_Original;
	FVector RH_IndexLoc_Original;
	FVector LH_IndexLoc_Original;
	FFingerRots_t RH_FingerRots_Original;
	FFingerRots_t LH_FingerRots_Original;
	FRotator Spine_01_Rot_Original;
	FRotator Head_Rot_Original;

	void ClearJointFlags() {	

		bSet_RH_Loc     = false;
		bSet_RH_Rot     = false;
		bUnSet_RH_Loc   = false;
		bUnSet_RH_Rot   = false;
		bSet_LH_Loc     = false;
		bSet_LH_Rot     = false;
		bUnSet_LH_Loc   = false;
		bUnSet_LH_Rot   = false;
		bSet_LI_Loc     = false;
		bSet_LF_Rot     = false;
		bSet_RI_Loc     = false;
		bSet_RF_Rot     = false;
		bUnSet_LI_Loc   = false;
		bUnSet_LF_Rot   = false;
		bUnSet_RI_Loc   = false;
		bUnSet_RF_Rot   = false;
		bSet_S01_Rot    = false;
		bUnSet_S01_Rot  = false;
		bSet_Head_Rot   = false;
		bUnSet_Head_Rot = false;

		bUsingRightHand = false;
		bUsingLeftHand  = false;
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIAIAVATAR_API UTaskAnimParamLogic : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTaskAnimParamLogic();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingBreadAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingBreadAnimRotCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingBreadAnimCurve_LH;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingBreadAnimRotCurve_LH;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingBreadAnimRotCurve_Spine01;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingSteakAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingSteakAnimRotCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingZucchiniAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* CuttingZucchiniAnimRotCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* SpooningSoupAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* SpooningSoupAnimRotCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* ForkingAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* ForkingAnimRotCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* ForkingAnimSpineRotCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* PouringAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* PouringAnimRotCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* PassingPageAnimLocCurve_Hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* PassingPageAnimRotCurve_Hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveFloat* PassingPageAnimRotCurve_Fingers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* PassingPageAnimRotCurve_Spine01;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* ReachingAnimLocCurve_Hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* ReachingAnimRotCurve_Hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveFloat* ReachingAnimRotCurve_Fingers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* ReachingAnimRotCurve_Head;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* ReachingAnimRotCurve_Spine01;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* DroppingAnimLocCurve_Hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* DroppingAnimRotCurve_Hand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveFloat* DroppingAnimRotCurve_Fingers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* DroppingAnimRotCurve_Head;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* DroppingAnimRotCurve_Spine01;

	// This tables are intended for the use of real human motion curves
	UDataTable *RH_Table;
	UDataTable *RH_Rot_Table;
	UDataTable *LH_Table;
	UDataTable *LH_Rot_Table;
	UDataTable *S1_Rot_Table;

	FRunAnimChainDelegate AnimChain;

	float writeTime;
	bool recorded;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	AIAIAvatarCharacter *Avatar;

	UIAIAvatarAnimationInstance *Animation;

	FTaskAnimParameters_t AnimParams;

	bool bRunAnimation;

	float currentAnimTime;
	float speedFactor;

	int pendingStates;

public:	

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	

	// ****** Help Functions ****** //
	void SetJointAlphas();
	void UnSetJointAlphas();

	// Check for item within a list of unique hit results and filter out those out of proper reach
	AActor* CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName);

	// News paper help function for holding locations
	FVector CalculateReachBookLocation(AActor* Book, FName Tag, bool bWorldRelative);

	// Getting current finger Rotations
	FFingerRots_t GetCurrentFingersRots(bool isRightHand);

	void SaveOriginalPose();

	// ****** Cutting Help Functions ****** //

	// Check for cuttable items within a list of unique hit results and filter out those out of proper reach
	TArray<AActor*> CheckForCuttableObjects(TMap<FString, FHitResult> Objects);

	// This will choose the biggest cuttable object 
	AActor* PickOneObject(TArray<AActor*> Cuttables);

	// Check if item is in good position for cutting
	bool isInGoodAlignment(ObjectData_t &ItemData);


	// ****** Setting Chain Animations ****** //

	// Pointing Book
	void StartPointBookAnimChain(AActor *Target);

	// Passing Page
	void StartPassPageAnimChain(AActor *Target);

	// Closing book
	void StartCloseBookAnimChain(AActor * Target);

	// Slicing
	void StartSlicingAnimChain(AActor* Target);

	// Grasping Object
	void StartGraspingAnimChain(FString Type, AActor *Target, FString Hand);

	// Placing Object
	void StartPlacingAnimChain(FString Type, FString Hand, FVector Point = FVector(0, 0, 0));

	// ****** Running Chains ****** //

	// Poiting Book
	void RunPointBookAnimChain(int stage);

	// Passing Page
	void RunPassPageAnimChain(int stage);

	// Closing book
	void RunCloseBookAnimChain(int stage);

	// Slicing
	void RunSlicingAnimChain(int stage);

	// Grasping Object
	void RunGraspingAnimChain(int stage);

	// Placing Object
	void RunPlacingAnimChain(int stage);

	// ****** Setting Parameters ****** //

	// Set parameters for finger reach animation
	void StartFingerReachAnimation(AActor *Target, FString Hand, FVector Point = FVector(0, 0, 0));

	// Set parameters for reach animation
	void StartReachAnimation(FString Type, AActor *Target, FString Hand, FVector Point = FVector(0,0,0));

	// Set aprameters for pass page animation. Assumes hand is already in position and Reach Animation have been run
	void StartPassPageAnimation();

	// Set parameters for drop animation
	void StartFingerReleaseAnimation(FString Hand);

	// Set parameters for drop animation
	void StartReleaseAnimation(FString Type, FString Hand);

	// Set parameters for cut animation
	void StartCutAnimation(ObjectData_t &ItemData);

	// Set parameters for pour animation
	void StartPourAnimation(AActor* Target);

	// Set parameters for fork animation
	void StartForkAnimation(AActor* Target);

	// Set parameters for Spoon animation
	void StartSpoonAnimation(AActor* Target);

	// ****** Running Animations ****** //

	// Running reach animation
	void RunReachAnimation(float time);

	// Running Pour Animation
	void RunPourAnimation(float time);

	// Running fork Animation
	void RunForkAnimation(float time);

	// Running fork Animation
	void RunSpoonAnimation(float time);

	// Running fork Animation
	void RunCutAnimation(float time);

	// ****** Processing Task ****** //

	// Process a task request
	void ProcessTask(FString task);

	// Process a task request on specific object
	void ApplyTaskOnActor(FString task, AActor* Object);

	// Process a task plus oject name
	void ProcessTask_P_ObjectName(FString task, FString ObjectName);

	void WriteCSV(float time);
};
