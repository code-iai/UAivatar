// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSPublisher.h"
#include "IAIAvatarCharacter.h"
#include "IAIAvatarAnimationInstance.h"
#include "Controller/AvatarConsoleCommandController.h"
#include "tf2_msgs/TFMessage.h"
#include "geometry_msgs/TransformStamped.h"

#include "TFBonePublisher.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UTFBonePublisher : public UROSPublisher
{
	GENERATED_BODY()

public:
	virtual void Publish();

	UPROPERTY(EditAnywhere)
	FString MapFrameId;

protected:
	virtual void SetMessageType();
	virtual void SetOwner(UObject* InOwner);

	UPROPERTY()
	UAvatarConsoleCommandController* Owner;

private:
	uint32 Seq;
};
