// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSPublisher.h"
#include "Controller/AvatarPerceiveObjController.h"
#include "Controller/AControllerComponent.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "urobosim_msgs/PerceiveObjectActionResult.h"
#include "AvatarPerceiveObjActionResultPublisher.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAvatarPerceiveObjActionResultPublisher : public UROSPublisher
{
	GENERATED_BODY()

public:
	virtual void Publish();

protected:
	virtual void SetMessageType();
	virtual void SetOwner(UObject* InOwner);

	UPROPERTY()
		UAvatarPerceiveObjController* Owner;
};
