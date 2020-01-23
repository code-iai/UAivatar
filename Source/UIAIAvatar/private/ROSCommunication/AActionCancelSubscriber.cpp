// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "AActionCancelSubscriber.h"

void UAActionCancelSubscriber::SetMessageType()
{
	MessageType = TEXT("actionlib_msgs/GoalID");
}

void UAActionCancelSubscriber::CreateSubscriber()
{
	Subscriber = MakeShareable<FAActionCancelCallback>(
		new FAActionCancelCallback(Topic, MessageType, ControllerComponent->Controller.ControllerList[ControllerName]));
	
	if (Subscriber.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Action Cancel Subscriber connected to RosBridge"));
	}
}