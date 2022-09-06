// Copyright 2022, Institute for Artificial Intelligence - University of Bremen
// Author: Mona Abdel-Keream (abdelker@uni-bremen.de)

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSSubscriber.h"
#include "ROSCommunication/JointStateBoneSubscriberCallback.h"
#include "JointStateBoneSubscriber.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UJointStateBoneSubscriber : public UROSSubscriber
{
	GENERATED_BODY()
public:

	virtual void SetMessageType();
	virtual void CreateSubscriber();
};
