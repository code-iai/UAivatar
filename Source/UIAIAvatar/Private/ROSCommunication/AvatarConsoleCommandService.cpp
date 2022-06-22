// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/AvatarConsoleCommandService.h"

void UAvatarConsoleCommandService::SetOwner(UObject* InOwner)
{
	Owner = Cast<UAControllerComponent>(InOwner);
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("No owner for Console Command Service"));
	}
}

void UAvatarConsoleCommandService::CreateServiceServer()
{
	ServiceServer = MakeShareable<FAvatarConsoleCommandsServer>(
		new FAvatarConsoleCommandsServer(Name, Type, Owner->Controller.ControllerList[ControllerName]));
}

void UAvatarConsoleCommandService::SetType()
{
	Type = TEXT("iai_avatar_msgs/Command");
}