// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeHandler.h"
#include "ROSBridgePublisher.h"
#include "ROSCommunication/ROSClient.h"
#include "Conversions.h"
#include "ROSPublisher.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UROSPublisher : public UObject
{
	GENERATED_BODY()
public:


	UROSPublisher();
	virtual void DeInit();
	virtual void Publish(float DeltaTime) {};
	virtual void Publish() {};

	virtual void Init(UObject* InOwner, TSharedPtr<FROSBridgeHandler> InHandler, FString InRosTopic);
	virtual void Init(FString InHostIp, uint32 InPort, UObject* InOwner);

	TSharedPtr<FROSBridgePublisher> Publisher;
	int Seq;

	UPROPERTY()
		FString ControllerName;

protected:


	UPROPERTY(EditAnywhere)
		FString Topic;

	UPROPERTY()
		FString MessageType;

	TSharedPtr<FROSBridgeHandler> Handler;

	virtual void SetMessageType() {};
	virtual void SetOwner(UObject* InOwner) {};
	virtual void CreatePublisher();
	bool skip(float Deltatime);

	/** Max Publishing Frequency (Hz). Use 0 for no limit*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float maxFrequency;

	float timekeeper;
};

