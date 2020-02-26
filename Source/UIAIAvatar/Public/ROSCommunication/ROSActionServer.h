// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeHandler.h"
#include "ROSCommunication/ROSPublisher.h"
#include "ROSCommunication/ROSSubscriber.h"
#include "ROSActionServer.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UROSActionServer : public UObject
{
	GENERATED_BODY()
public:

	UROSActionServer() {};

	virtual void Init(TSharedPtr<FROSBridgeHandler> InHandler, UObject* InOwner, FString InActionName = "");
	virtual void Init(FString InHostIp, uint32 InPort, UObject* InOwner, FString InActionName = "");

	virtual void Tick(float DeltaTime);

	UPROPERTY(EditAnywhere)
		FString ControllerName;

protected:

	UPROPERTY(EditAnywhere)
		UROSPublisher* FeedbackPublisher;

	UPROPERTY(EditAnywhere)
		UROSPublisher* StatusPublisher;

	UPROPERTY(EditAnywhere)
		UROSPublisher* ResultPublisher;

	UPROPERTY(EditAnywhere)
		UROSSubscriber* GoalSubscriber;

	UPROPERTY(EditAnywhere)
		UROSSubscriber* CancelSubscriber;

	TSharedPtr<FROSBridgeHandler> Handler;

	virtual void Init();

	UPROPERTY()
		UObject* Owner;

	UPROPERTY(EditAnywhere)
		FString ActionName;
};
