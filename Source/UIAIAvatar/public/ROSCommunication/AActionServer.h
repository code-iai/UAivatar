// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeHandler.h"
#include "ROSCommunication/APublisher.h"
#include "ROSCommunication/ASubscriber.h"
#include "AActionServer.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UAActionServer : public UObject
{
	GENERATED_BODY()
public:

	UAActionServer() {};

	virtual void Init(TSharedPtr<FROSBridgeHandler> InHandler, UObject* InOwner, FString InActionName = "");
	virtual void Init(FString InHostIp, uint32 InPort, UObject* InOwner, FString InActionName = "");

	virtual void Tick();

	UPROPERTY(EditAnywhere)
		FString ControllerName;

protected:

	UPROPERTY(EditAnywhere)
		UAPublisher* FeedbackPublisher;

	UPROPERTY(EditAnywhere)
		UAPublisher* StatusPublisher;

	UPROPERTY(EditAnywhere)
		UAPublisher* ResultPublisher;

	UPROPERTY(EditAnywhere)
		UASubscriber* GoalSubscriber;

	UPROPERTY(EditAnywhere)
		UASubscriber* CancelSubscriber;

	TSharedPtr<FROSBridgeHandler> Handler;

	virtual void Init();

	UPROPERTY()
		UObject* Owner;

	UPROPERTY(EditAnywhere)
		FString ActionName;
};
