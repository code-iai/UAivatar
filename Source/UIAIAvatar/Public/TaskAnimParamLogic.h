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

struct CuttableObjectData_t
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

	// Right Hand
	UCurveVector* RH_Loc_Curve;
	DataTableHandler* RH_Loc_Table;
	FVector RH_Loc_Multiplier;
	FVector RH_Loc_Offset;
	FRotator RH_Loc_Curve_Orientation;

	UCurveVector* RH_Rot_Curve;
	DataTableHandler* RH_Rot_Table;

	bool bSet_RH_Loc;
	bool bSet_RH_Rot;

	// Left Hand
	UCurveVector* LH_Loc_Curve;
	DataTableHandler* LH_Loc_Table;
	FVector LH_Loc_Multiplier;
	FVector LH_Loc_Offset;

	FRotator LH_Rotation;
	UCurveVector* LH_Rot_Curve;
	DataTableHandler* LH_Rot_Table;

	bool bSet_LH_Loc;
	bool bSet_LH_Rot;

	// Fingers
	FFingerRots_t RH_FingerRots;
	FFingerRots_t LH_FingerRots;

	bool bSet_LF_Rot;
	bool bSet_RF_Rot;

	// Spine
	FRotator Spine_01_rotation;
	UCurveVector* Spine01_Rot_Curve;
	DataTableHandler* Spine01_Rot_Table;

	bool bSet_S01_Rot;

	float animTime;
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
		UCurveVector* CuttingBreadAnimSpineRotCurve;

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
	
	UDataTable *RH_Table;
	UDataTable *RH_Rot_Table;
	UDataTable *LH_Table;
	UDataTable *LH_Rot_Table;
	UDataTable *S1_Rot_Table;

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
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	

	// ****** Help Functions ****** //
	void SetJointAlphas();
	void UnSetJointAlphas();

	// Check for item within a list of unique hit results and filter out those out of proper reach
	AActor* CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName);

	// ****** Cutting Help Functions ****** //

	// Check for cuttable items within a list of unique hit results and filter out those out of proper reach
	TArray<AActor*> CheckForCuttableObjects(TMap<FString, FHitResult> Objects);

	// This will choose the biggest cuttable object 
	AActor* PickOneObject(TArray<AActor*> Cuttables);

	// Check if item is in good position for cutting
	bool isInGoodAlignment(CuttableObjectData_t &ItemData);

	// ****** Setting Parameters ****** //

	// Set parameters for cut animation
	void calculateCutAnimParameters(CuttableObjectData_t &ItemData);

	// Set parameters for pour animation
	void calculatePourAnimParameters(AActor* Target);

	// Set parameters for fork animation
	void calculateForkAnimParameters(AActor* Target);

	// Set parameters for Spoon animation
	void calculateSpoonAnimParameters(AActor* Target);

	// ****** Running Animations ****** //

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
