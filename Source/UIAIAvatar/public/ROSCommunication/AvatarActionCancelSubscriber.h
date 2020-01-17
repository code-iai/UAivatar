// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ASubscriber.h"
#include "ROSCommunication/AvatarActionCancelSubscriberCallback.h"
#include "AvatarActionCancelSubscriber.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAvatarActionCancelSubscriber : public UASubscriber
{
	GENERATED_BODY()
public:
	virtual void SetMessageType();
	virtual void CreateSubscriber();
};
