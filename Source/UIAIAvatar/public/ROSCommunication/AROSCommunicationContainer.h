#pragma once

// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "Components/ActorComponent.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgePublisher.h"
#include "ROSCommunication/APublisher.h"
#include "ROSCommunication/ASubscriber.h"
#include "ROSCommunication/AROSClient.h"
#include "ROSCommunication/AService.h"
#include "ROSCommunication/AActionServer.h"
#include "AROSCommunicationContainer.generated.h"

class UAControllerComponent;

USTRUCT()
struct FROSTopic
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		FString Topic;

	UPROPERTY(EditAnywhere)
		FString Type;

	int32 ID;

	FROSTopic(FString InTopic = "", FString InType = "") : Topic(InTopic), Type(InType) {};

};


USTRUCT(Blueprintable)
struct FAROSCommunicationContainer
{
	GENERATED_BODY()

public:

	TSharedPtr<FROSBridgeHandler> Handler;

	// UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, export, noclear)
	// URRosComunicationInterface* Interface;

	UPROPERTY()
	UAControllerComponent* ControllerComponent;

	UPROPERTY(EditAnywhere, Category = "ROS Bridge Agent")
		bool bUseGlobalHandler;

	UPROPERTY(EditAnywhere, Category = "ROS Bridge Agent")
		FString WebsocketIPAddr;

	UPROPERTY(EditAnywhere, Category = "ROS Bridge Agent")
		uint32 WebsocketPort;

	UPROPERTY(EditAnywhere, Category = "ROS Bridge Agent")
		FString RobotName;

	UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, export, noclear)
		TMap<FString, UAPublisher*> PublisherList;

	UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, export, noclear)
		TMap<FString, UASubscriber*> SubscriberList;

	UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, export, noclear)
		TMap<FString, UAROSClient*> ClientList;

	UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, export, noclear)
		TMap<FString, UAService*> ServiceProviderList;

	UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, export, noclear)
		TMap<FString, UAActionServer*> ActionServerList;

	FAROSCommunicationContainer();

	virtual ~FAROSCommunicationContainer() {};
	virtual void InitHandler();

	virtual void InitAllPublisher();
	virtual void InitAllSubscriber();
	virtual void InitAllServiceProvider();
	virtual void InitAllClients();
	virtual void InitAllActionServer();

	virtual void Init();
	virtual void DeInit();
	virtual void Tick(float DeltaTime);

protected:

};