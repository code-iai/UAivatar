// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSPublisher.h"
#include "Controller/AvatarQnAController.h"
#include "Controller/AControllerComponent.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "iai_avatar_msgs/actions/QnAActionResult.h"
//#include "AvatarQnAActionResultPublisher.generated.h"

/**
 
UCLASS()
class UIAIAVATAR_API UAvatarQnAActionResultPublisher : public UROSPublisher
{
	GENERATED_BODY()

public:
	virtual void Publish();

protected:
	virtual void SetMessageType();
	virtual void SetOwner(UObject* InOwner);

	UPROPERTY()
//		UAvatarQnAController* Owner;
};* 
 */