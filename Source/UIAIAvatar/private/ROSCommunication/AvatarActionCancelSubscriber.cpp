// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "AvatarActionCancelSubscriber.h"

void UAvatarActionCancelSubscriber::SetMessageType()
{
	MessageType = TEXT("actionlib_msgs/GoalID");
}

void UAvatarActionCancelSubscriber::CreateSubscriber()
{
	Subscriber = MakeShareable<FAvatarActionCancelCallback>(
		new FAvatarActionCancelCallback(Topic, MessageType, ControllerComponent->Controller.ControllerList[ControllerName]));
	
	if (Subscriber.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Action Cancel Subscriber connected to RosBridge"));
	}
}