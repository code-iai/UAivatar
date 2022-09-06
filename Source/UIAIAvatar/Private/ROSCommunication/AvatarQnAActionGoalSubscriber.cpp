// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "ROSCommunication/AvatarQnAActionGoalSubscriber.h"
/*
void UAvatarQnAActionGoalSubscriber::SetMessageType()
{
	MessageType = TEXT("iai_avatar_msgs/QnAActionGoal");
}

void UAvatarQnAActionGoalSubscriber::CreateSubscriber()
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ControllerName);
	
	Subscriber = MakeShareable<FAvatarQnAActionGoalCallback>(
		new FAvatarQnAActionGoalCallback(Topic, MessageType, ControllerComponent->Controller.ControllerList[ControllerName]));
	if (Subscriber.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("QnA Action Goal Subscriber connected to RosBridge"));
	}
}
*/