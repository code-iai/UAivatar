// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeHandler.h"
#include "ROSBridgePublisher.h"
#include "ROSBridgeSrvServer.h"
#include "ROSCommunication/AROSServiceServer.h"
#include "AService.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UAService : public UObject
{
	GENERATED_BODY()
public:
	// virtual void CallService(){};
	virtual void Init(UObject* InModel, TSharedPtr<FROSBridgeHandler> InHandler, FString InName);

	TSharedPtr<FROSBridgeSrvServer> ServiceServer;
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