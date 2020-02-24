// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "Components/ActorComponent.h"
#include "IAIAvatarAnimationInstance.h"
#include "TaskAnimParamLogic.generated.h"

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FString Task;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		bool success;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* RH_Curve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FVector RH_Curve_Offset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FVector RH_Curve_Multiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator RH_Curve_Orientation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* RH_Rotation_Curve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FVector LH_Location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator LH_Rotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FFingerRots_t RH_FingerRots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FFingerRots_t LH_FingerRots;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
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
		UCurveVector* PouringAnimCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		UCurveVector* PouringAnimRotCurve;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	AIAIAvatarCharacter *Avatar;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Check for cuttable items within a list of unique hit results and filter out those out of proper reach
	TArray<AActor*> CheckForCuttableObjects(TMap<FString, FHitResult> Objects);
	
	UFUNCTION(BlueprintCallable)
	// Check for item within a list of unique hit results and filter out those out of proper reach
	AActor* CheckForObject(TMap<FString, FHitResult> Objects, FString ObjName);

	// This will choose the biggest cuttable object 
	AActor* PickOneObject(TArray<AActor*> Cuttables);

	// Check if item is in good position for cutting
	bool isInGoodAlignment(CuttableObjectData_t &ItemData);

	// Set parameters for task animation
	FTaskAnimParameters_t calculateCutAnimParameters(CuttableObjectData_t &ItemData);

	// Set parameters for task animation
	FTaskAnimParameters_t calculatePourAnimParameters(AActor* Target);

	// Process a task request
	UFUNCTION(BlueprintCallable)
	FTaskAnimParameters_t processTask(FString task);

	// Process a task request on specific object
	UFUNCTION(BlueprintCallable)
	FTaskAnimParameters_t processTaskOn(FString task, AActor* Object);
};
