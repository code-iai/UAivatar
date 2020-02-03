// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/ROSPublisher.h"
#include "actionlib_msgs/GoalStatusArray.h"

UROSPublisher::UROSPublisher()
{
	maxFrequency = 0;
	timekeeper = 0;
}

void UROSPublisher::DeInit()
{
	if (Handler.IsValid()) {
		Handler->Disconnect();
	}
}

void UROSPublisher::Init(FString InHostIp, uint32 InPort, UObject* InOwner)
{
	Seq = 0;
	Handler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(InHostIp, InPort));
	Handler->Connect();
	SetMessageType();
	SetOwner(InOwner);
	CreatePublisher();
	Handler->AddPublisher(Publisher);
}

void UROSPublisher::Init(UObject* InOwner, TSharedPtr<FROSBridgeHandler> InHandler, FString InRosTopic)
{
	Seq = 0;
	Handler = InHandler;
	if (!InRosTopic.Equals(""))
	{
		Topic = InRosTopic;
	}
	SetMessageType();
	SetOwner(InOwner);
	CreatePublisher();
	Handler->AddPublisher(Publisher);
}

void UROSPublisher::CreatePublisher()
{
	Publisher = MakeShareable<FROSBridgePublisher>
		(new FROSBridgePublisher(Topic, MessageType));

	if (Publisher.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Publisher connected to RosBridge"));
	}
}

bool UROSPublisher::skip(float DeltaTime) {
	
	if (maxFrequency != 0) {
		
		bool skip = true;
		float period = 1 / maxFrequency;
		if (timekeeper >= period) {
			timekeeper = 0;
		}

		if (!timekeeper)
			skip = false;

		timekeeper += DeltaTime;

		return skip;
	}

	return false;
}