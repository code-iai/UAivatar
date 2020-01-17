// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author:Michael Neumann

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSubscriber.h"
#include "Controller/AController.h"


class UIAIAVATAR_API FAvatarActionCancelCallback : public FROSBridgeSubscriber
{
public:
	FAvatarActionCancelCallback(const FString& InTopic, const FString& InType, UObject* InController);

	~FAvatarActionCancelCallback() override;

	TSharedPtr<FROSBridgeMsg> ParseMessage(TSharedPtr<FJsonObject> JsonObject) const override;

	void Callback(TSharedPtr<FROSBridgeMsg> Msg) override;

	UAController* Controller;

};