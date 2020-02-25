// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "Controller/AController.h"
#include "AvatarConsoleCommandController.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UAvatarConsoleCommandController : public UAController
{
	GENERATED_BODY()
public:
	UAvatarConsoleCommandController() {};

	virtual void Init(AActor* InAgent) override;

	virtual void SendConsoleCommand();

	virtual void Tick(float InDeltaTime) override;

	UPROPERTY()
		FString Command;

	UPROPERTY()
		FString Message;

	UPROPERTY()
		bool bSuccess;

	UPROPERTY()
		AIAIAvatarCharacter* Avatar;
};