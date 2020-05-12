// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ROSUtilities.h"
#include "AController.generated.h"

/**
 *
 */

// USTRUCT()
// struct FGoalStatusInfo
// {
// 	GENERATED_BODY()
// public:
// 	FGoalStatusInfo() {};

// 	FGoalStatusInfo(FString InId, uint32 InSecs, uint32 InNSecs, uint8 InStatus = 0, FString InText = "") : Id(InId),
// 		Secs(InSecs),
// 		NSecs(InNSecs),
// 		Status(InStatus),
// 		Text(InText) {};

// 	FString Id;
// 	uint32 Secs;
// 	uint32 NSecs;
// 	uint8 Status;
// 	FString Text;

// };

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UAController : public UObject
{
	GENERATED_BODY()
public:

	virtual void Tick(float InDeltaTime) {};
	virtual void Init(AActor* Agent) {};
	virtual void CancelAction();

	UPROPERTY()
		bool bActive;

	UPROPERTY()
		bool bCancel = false;

	UPROPERTY()
		bool bPublishResult = false;

	UPROPERTY()
		TArray<FGoalStatusInfo> GoalStatusList;

	UPROPERTY()
		float CummulatedError = 0;

};
