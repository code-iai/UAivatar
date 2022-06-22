// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "Controller/AController.h"
#include "AvatarSemanticCameraController.generated.h"


USTRUCT(Blueprintable)
struct FSemanticObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FString Name;
	UPROPERTY()
		TArray<FString> Tags;
};


/**
 * 
 */
UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UAvatarSemanticCameraController : public UAController
{
	GENERATED_BODY()
public:
	UAvatarSemanticCameraController() {};

	virtual void Init(AActor* InAgent) override;

	virtual void ListObjects();

	virtual void Tick(float InDeltaTime) override;

	UPROPERTY()
		TArray<FSemanticObject> Objects;

	UPROPERTY()
		FString Message;

	UPROPERTY()
		bool bSuccess;

protected:

	UPROPERTY()
		AIAIAvatarCharacter* Avatar;
};