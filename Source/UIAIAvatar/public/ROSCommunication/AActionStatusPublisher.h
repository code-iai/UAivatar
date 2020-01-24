// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/APublisher.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "Controller/AController.h"
#include "Controller/AControllerComponent.h"
#include "AActionStatusPublisher.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAActionStatusPublisher : public UAPublisher
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
