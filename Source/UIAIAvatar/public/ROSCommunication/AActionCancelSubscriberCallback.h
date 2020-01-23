// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author:Michael Neumann

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSubscriber.h"
#include "Controller/AController.h"


class UIAIAVATAR_API FAActionCancelCallback : public FROSBridgeSubscriber
{
public:
	FAActionCancelCallback(const FString& InTopic, const FString& InType, UObject* InController);

	~FAActionCancelCallback() override;

	TSharedPtr<FROSBridgeMsg> ParseMessage(TSharedPtr<FJsonObject> JsonObject) const override;

	void Callback(TSharedPtr<FROSBridgeMsg> Msg) override;

	UAController* Controller;

};