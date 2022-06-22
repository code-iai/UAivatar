#include "ROSCommunication/ROSCommunicationContainer.h"
#include "Controller/AControllerComponent.h"

FROSCommunicationContainer::FROSCommunicationContainer()
{
	WebsocketIPAddr = TEXT("127.0.0.1");
	WebsocketPort = 9090;
	RobotName = TEXT("agent");
}


void FROSCommunicationContainer::Init()
{
	InitHandler();
	if (Handler.IsValid())
	{
		Handler->Connect();
		InitAllPublisher();
		InitAllSubscriber();
		InitAllServiceProvider();
		InitAllClients();
		InitAllActionServer();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No FROSBridgeHandler created."));
	}
}


void FROSCommunicationContainer::InitAllServiceProvider()
{
	for (auto& ServiceProvider : ServiceProviderList)
	{
		ServiceProvider.Value->Init(ControllerComponent, Handler, ServiceProvider.Key);
	}
}

void FROSCommunicationContainer::InitAllPublisher()
{
	for (auto& Publisher : PublisherList)
	{
		Publisher.Value->Init(WebsocketIPAddr, WebsocketPort, ControllerComponent->GetOwner());
	}
}

void FROSCommunicationContainer::InitAllSubscriber()
{
	for (auto& Subscriber : SubscriberList)
	{
		Subscriber.Value->Init(ControllerComponent->GetOwner(), Handler);
	}
}

void FROSCommunicationContainer::InitAllClients()
{
	for (auto& Client : ClientList)
	{
		Client.Value->Init(ControllerComponent->GetOwner(), Handler, RobotName + "/" + Client.Key);
	}
}

void FROSCommunicationContainer::InitAllActionServer()
{
	for (auto& ActionServer : ActionServerList)
	{
		ActionServer.Value->Init(WebsocketIPAddr, WebsocketPort, ControllerComponent->GetOwner());
	}
}

void FROSCommunicationContainer::Tick(float DeltaTime)
{

	if (Handler.IsValid())
	{
		for (auto& Publisher : PublisherList)
		{
			Publisher.Value->Publish();
		}

		for (auto& ActionServer : ActionServerList)
		{
			ActionServer.Value->Tick(DeltaTime);
		}

		Handler->Process();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Handler Invalid"));
	}
}

void FROSCommunicationContainer::InitHandler()
{
	if (bUseGlobalHandler)
	{
		//TODO implement use global handler
		UE_LOG(LogTemp, Error, TEXT("Global handler not supported yet"));
	}
	else
	{
		Handler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(WebsocketIPAddr, WebsocketPort));
	}
}

void FROSCommunicationContainer::DeInit()
{
	//Disconnect the handler before parent ends
	if (bUseGlobalHandler)
	{
		//TODO implement use global handler
		UE_LOG(LogTemp, Error, TEXT("Global handler not supported yet"));
	}
	else
	{
		for (auto& Publisher : PublisherList)
		{
			Publisher.Value->DeInit();
		}
		if (Handler.IsValid())
		{
			Handler->Disconnect();
		}
	}

}
