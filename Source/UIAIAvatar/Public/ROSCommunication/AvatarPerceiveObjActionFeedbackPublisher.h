// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSPublisher.h"
#include "Controller/AvatarPerceiveObjController.h"
#include "Controller/AControllerComponent.h"
#include "urobosim_msgs/PerceiveObjectActionFeedback.h"
#include "AvatarPerceiveObjActionFeedbackPublisher.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAvatarPerceiveObjActionFeedbackPublisher : public UROSPublisher
{
	GENERATED_BODY()
	
public:

	virtual void Publish(float DeltaTime);

protected:
	virtual void SetMessageType();
	virtual void SetOwner(UObject* InAgent);

	UPROPERTY()
		UAvatarPerceiveObjController* Owner;
};
