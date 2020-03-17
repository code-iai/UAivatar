// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "Components/ActorComponent.h"
#include "IAIAvatarAnimationInstance.h"
#include "Curves/CurveVector.h"
#include "TaskAnimParamLogic.generated.h"


DECLARE_DELEGATE_OneParam(FRunAnimDelegate, float);

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FString Task;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		bool success;

	FRunAnimDelegate AnimFunctionDelegate;
	TArray<bool> ActiveActuators;

	// Right Hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* RH_Loc_Curve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FVector RH_Loc_Curve_Offset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FVector RH_Loc_Curve_Multiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator RH_Loc_Curve_Orientation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* RH_Rot_Curve;

	bool bSet_RH_Loc;
	bool bSet_RH_Rot;

	// Left Hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FVector LH_Location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator LH_Rotation;

	bool bSet_LH_Loc;
	bool bSet_LH_Rot;

	// Fingers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FFingerRots_t RH_FingerRots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FFingerRots_t LH_FingerRots;

	bool bSet_LF_Rot;
	bool bSet_RF_Rot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		float animTime;

	// Spine
	FRotator Spine_01_rotation;
	bool bSet_S01_Rot;
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
		UCurveVector* PouringAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* PouringAnimRotCurve;

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

	UFUNCTION(BlueprintCallable)
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
	UFUNCTION(BlueprintCallable)
	FTaskAnimParameters_t ProcessTask(FString task);

	// Process a task request on specific object
	UFUNCTION(BlueprintCallable)
	FTaskAnimParameters_t ApplyTaskOnActor(FString task, AActor* Object);

	// Process a task plus oject name
	UFUNCTION(BlueprintCallable)
		void ProcessTask_P_ObjectName(FString task, FString ObjectName);

};
