// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "ROSCommunication/ROSActionCancelSubscriber.h"

void UROSActionCancelSubscriber::SetMessageType()
{
	MessageType = TEXT("actionlib_msgs/GoalID");
}

void UROSActionCancelSubscriber::CreateSubscriber()
{
	Subscriber = MakeShareable<FROSActionCancelCallback>(
		new FROSActionCancelCallback(Topic, MessageType, ControllerComponent->Controller.ControllerList[ControllerName]));
	
	if (Subscriber.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Action Cancel Subscriber connected to RosBridge"));
	}
}