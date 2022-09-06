// Copyright 2022, Institute for Artificial Intelligence - University of Bremen
// Author: Mona Abdel-Keream (abdelker@uni-bremen.de)


#include "ROSCommunication/JointStateBoneSubscriber.h"

void UJointStateBoneSubscriber::SetMessageType()
{
	MessageType = TEXT("sensor_msgs/JointState");
}

void UJointStateBoneSubscriber::CreateSubscriber()
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ControllerName);
	
	Subscriber = MakeShareable<FJointStateBoneSubscriberCallback>(
		new FJointStateBoneSubscriberCallback(Topic, MessageType, ControllerComponent->Controller.ControllerList[ControllerName]));
	if (Subscriber.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("JointState Subscriber connected to RosBridge"));
	}
}
