// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "IAIAvatarAnimationInstance.h"
#include "Controller/AController.h"
#include "Kismet/GameplayStatics.h"
#include "AvatarPerceiveObjController.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UAvatarPerceiveObjController : public UAController
{
	GENERATED_BODY()

public:
	UAvatarPerceiveObjController();

	virtual void Init(AActor* InAgent) override;
	virtual void Tick(float InDeltaTime) override;

	virtual void PerceiveObject();
	
	UPROPERTY()
		FString TypeToPerceive;

	UPROPERTY()
		FString ObjectName;

	UPROPERTY()
		FTransform ObjectPose;

	UPROPERTY()
		FTransform ObjectPoseWorld;

protected:

	UPROPERTY()
	AIAIAvatarCharacter* Avatar;

};
