// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/ROSClient.h"
#include "TimerManager.h"

void UROSClient::Init(UObject* InModel, TSharedPtr<FROSBridgeHandler> InHandler, FString InName)
{
	ROSHandler = InHandler;
	Init(InModel);
}

void UROSClient::Init(UObject* InModel, TArray<FString>* OutArray, TSharedPtr<FROSBridgeHandler> InHandler)
{
	ROSHandler = InHandler;
	Init(InModel, OutArray);
}

