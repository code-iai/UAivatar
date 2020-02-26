// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSSubscriber.h"
#include "ROSCommunication/ROSActionCancelSubscriberCallback.h"
#include "ROSActionCancelSubscriber.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UROSActionCancelSubscriber : public UROSSubscriber
{
	GENERATED_BODY()
public:
	virtual void SetMessageType();
	virtual void CreateSubscriber();
};
