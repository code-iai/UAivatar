// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/ROSSubscriber.h"
#include "GameFramework/Actor.h"

void UROSSubscriber::Init(UObject* InAgent, TSharedPtr<FROSBridgeHandler> InHandler, FString InRosTopic)
{
	AActor* Temp = Cast<AActor>(InAgent);
	ControllerComponent = Temp->FindComponentByClass<UAControllerComponent>();

	Init(InRosTopic);
	InHandler->AddSubscriber(Subscriber);

}

void UROSSubscriber::Init(FString RosTopic)
{
	if (!RosTopic.Equals(""))
	{
		Topic = RosTopic;
	}
	SetMessageType();
	CreateSubscriber();
}