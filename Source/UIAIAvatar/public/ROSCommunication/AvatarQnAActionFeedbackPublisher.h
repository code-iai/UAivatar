// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/APublisher.h"
#include "Controller/AvatarQnAController.h"
#include "Controller/AControllerComponent.h"
#include "iai_avatar_msgs/actions/QnAActionFeedback.h"
#include "AvatarQnAActionFeedbackPublisher.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAvatarQnAActionFeedbackPublisher : public UAPublisher
{
	GENERATED_BODY()
	
public:

	virtual void Publish();
protected:
	virtual void SetMessageType();
	virtual void SetOwner(UObject* InAgent);

	UPROPERTY()
		UAvatarQnAController* Owner;
};
