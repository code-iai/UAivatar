// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author:Michael Neumann

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSubscriber.h"
#include "iai_avatar_msgs/actions/QnAActionGoal.h"
#include "Controller/AvatarQnAController.h"

/*
class UIAIAVATAR_API FAvatarQnAActionGoalCallback : public FROSBridgeSubscriber
{
public:
	FAvatarQnAActionGoalCallback(const FString& InTopic, const FString& InType, UObject* InController);

	~FAvatarQnAActionGoalCallback() override;

	TSharedPtr<FROSBridgeMsg> ParseMessage(TSharedPtr<FJsonObject> JsonObject) const override;

	void Callback(TSharedPtr<FROSBridgeMsg> Msg) override;

//	UAvatarQnAController* Controller;
};*
*/