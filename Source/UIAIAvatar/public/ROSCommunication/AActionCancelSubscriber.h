// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ASubscriber.h"
#include "ROSCommunication/AActionCancelSubscriberCallback.h"
#include "AActionCancelSubscriber.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAActionCancelSubscriber : public UASubscriber
{
	GENERATED_BODY()
public:
	virtual void SetMessageType();
	virtual void CreateSubscriber();
};
