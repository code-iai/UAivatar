// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSSubscriber.h"
#include "ROSCommunication/AvatarPerceiveObjActionGoalSubscriberCallback.h"
#include "AvatarPerceiveObjActionGoalSubscriber.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAvatarPerceiveObjActionGoalSubscriber : public UROSSubscriber
{
	GENERATED_BODY()
public:

	virtual void SetMessageType();
	virtual void CreateSubscriber();
};
