// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/AROSClient.h"
#include "TimerManager.h"

void UAROSClient::Init(UObject* InModel, TSharedPtr<FROSBridgeHandler> InHandler, FString InName)
{
	ROSHandler = InHandler;
	Init(InModel);
}

void UAROSClient::Init(UObject* InModel, TArray<FString>* OutArray, TSharedPtr<FROSBridgeHandler> InHandler)
{
	ROSHandler = InHandler;
	Init(InModel, OutArray);
}

