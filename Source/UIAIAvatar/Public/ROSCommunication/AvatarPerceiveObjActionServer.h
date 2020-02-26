// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSActionServer.h"
#include "ROSCommunication/ROSActionStatusPublisher.h"
#include "ROSCommunication/ROSActionCancelSubscriber.h"
#include "ROSCommunication/AvatarPerceiveObjActionResultPublisher.h"
#include "ROSCommunication/AvatarPerceiveObjActionFeedbackPublisher.h"
#include "ROSCommunication/AvatarPerceiveObjActionGoalSubscriber.h"
#include "AvatarPerceiveObjActionServer.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UAvatarPerceiveObjActionServer : public UROSActionServer
{
	GENERATED_BODY()
public:
	UAvatarPerceiveObjActionServer();
};
