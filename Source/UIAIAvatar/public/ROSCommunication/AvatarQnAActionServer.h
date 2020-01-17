// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/AActionServer.h"
#include "ROSCommunication/AvatarActionCancelSubscriber.h"
#include "ROSCommunication/AvatarQnAActionResultPublisher.h"
#include "ROSCommunication/AvatarQnAActionGoalSubscriber.h"
#include "AvatarQnAActionServer.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UAvatarQnAActionServer : public UAActionServer
{
	GENERATED_BODY()
public:
	UAvatarQnAActionServer();
};
