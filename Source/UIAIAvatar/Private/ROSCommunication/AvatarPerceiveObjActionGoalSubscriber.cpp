// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "ROSCommunication/AvatarPerceiveObjActionGoalSubscriber.h"

void UAvatarPerceiveObjActionGoalSubscriber::SetMessageType()
{
	MessageType = TEXT("urobosim_msgs/PerceiveObjectActionGoal");
}

void UAvatarPerceiveObjActionGoalSubscriber::CreateSubscriber()
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ControllerName);
	
	Subscriber = MakeShareable<FAvatarPerceiveObjActionGoalCallback>(
		new FAvatarPerceiveObjActionGoalCallback(Topic, MessageType, ControllerComponent->Controller.ControllerList[ControllerName]));
	if (Subscriber.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Perceive Object Action Goal Subscriber connected to RosBridge"));
	}
}