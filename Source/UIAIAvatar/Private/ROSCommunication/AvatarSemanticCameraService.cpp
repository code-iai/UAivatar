// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "ROSCommunication/AvatarSemanticCameraService.h"

void UAvatarSemanticCameraService::SetOwner(UObject* InOwner)
{
	Owner = Cast<UAControllerComponent>(InOwner);
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for Semantic Camera Service"));
	}
}

void UAvatarSemanticCameraService::CreateServiceServer()
{
	ServiceServer = MakeShareable<FAvatarSemanticCameraServer>(
		new FAvatarSemanticCameraServer(Name, Type, Owner->Controller.ControllerList[ControllerName]));
}

void UAvatarSemanticCameraService::SetType()
{
	Type = TEXT("iai_avatar_msgs/SemanticCamera");
}