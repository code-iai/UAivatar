// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/AService.h"

void UAService::Init(UObject* InOwner, TSharedPtr<FROSBridgeHandler> InHandler, FString InName)
{
	ROSHandler = InHandler;
	SetOwner(InOwner);
	SetType();
	CreateServiceServer();
	ROSHandler->AddServiceServer(ServiceServer);
}

