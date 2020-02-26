// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeHandler.h"
#include "ROSBridgePublisher.h"
#include "ROSBridgeSrvServer.h"
#include "ROSService.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UROSService : public UObject
{
	GENERATED_BODY()
public:
	// virtual void CallService(){};
	virtual void Init(UObject* InModel, TSharedPtr<FROSBridgeHandler> InHandler, FString InName);

	TSharedPtr<FROSBridgeSrvServer> ServiceServer;

	UPROPERTY(EditAnywhere)
		FString ControllerName;

protected:

	UPROPERTY(EditAnywhere, Category = "ROS Service Server")
		FString Name;

	UPROPERTY()
		FString Type;

	TSharedPtr<FROSBridgeHandler> ROSHandler;

	virtual void SetOwner(UObject* Owner) {};
	virtual void SetType() {};
	virtual void CreateServiceServer() {};
};