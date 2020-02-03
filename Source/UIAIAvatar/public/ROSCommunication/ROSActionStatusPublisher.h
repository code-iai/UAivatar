// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSPublisher.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "Controller/AController.h"
#include "Controller/AControllerComponent.h"
#include "ROSActionStatusPublisher.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UROSActionStatusPublisher : public UROSPublisher
{
	GENERATED_BODY()
	
public:

	virtual void Publish(float DeltaTime);

protected:
	virtual void SetMessageType();
	virtual void SetOwner(UObject* InOwner);

	UPROPERTY()
		UAController* Owner;
};
