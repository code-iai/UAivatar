// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/ROSActionServer.h"

void UROSActionServer::Init(TSharedPtr<FROSBridgeHandler> InHandler, UObject* InOwner, FString InActionName)
{
	Handler = InHandler;
	Owner = InOwner;
	if (!InActionName.Equals(""))
	{
		ActionName = InActionName;
	}
	Init();
}

void UROSActionServer::Init(FString InHostIp, uint32 InPort, UObject* InOwner, FString InActionName)
{
	Handler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(InHostIp, InPort));
	Handler->Connect();
	Owner = InOwner;
	if (!InActionName.Equals(""))
	{
		ActionName = InActionName;
	}
	Init();
}

void UROSActionServer::Tick(float DeltaTime)
{
	if (Handler.IsValid())
	{
		Handler->Process();
	}

	if (FeedbackPublisher)
	{
		FeedbackPublisher->Publish(DeltaTime);
	}

	if (ResultPublisher)
	{
		ResultPublisher->Publish();
	}

	if (StatusPublisher)
	{
		StatusPublisher->Publish(DeltaTime);
	}
}

void UROSActionServer::Init()
{

	UE_LOG(LogTemp, Log, TEXT("Init Action Server %s"), *GetName());
	CancelSubscriber->ControllerName = ControllerName;
	StatusPublisher->ControllerName = ControllerName;
	ResultPublisher->ControllerName = ControllerName;
	FeedbackPublisher->ControllerName = ControllerName;
	GoalSubscriber->ControllerName = ControllerName;

	CancelSubscriber->Init(Owner, Handler, ActionName + "/cancel");
	GoalSubscriber->Init(Owner, Handler, ActionName + "/goal");
	FeedbackPublisher->Init(Owner, Handler, ActionName + "/feedback");
	ResultPublisher->Init(Owner, Handler, ActionName + "/result");
	StatusPublisher->Init(Owner, Handler, ActionName + "/status");
}



