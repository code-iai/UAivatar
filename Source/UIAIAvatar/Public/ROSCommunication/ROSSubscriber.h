// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeHandler.h"
#include "Controller/AControllerComponent.h"
#include "ROSSubscriber.generated.h"

//class URControllerComponent;

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UROSSubscriber : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere)
		FString Topic;

	//UPROPERTY()
		//ARModel* Model;

	UPROPERTY()
	UAControllerComponent* ControllerComponent;

	virtual void Init(UObject* InAgent, TSharedPtr<FROSBridgeHandler> InHandler, FString InRosTopic = "");

	virtual void Init(UObject* InAgent) {};

	TSharedPtr<FROSBridgeSubscriber> Subscriber;

	UPROPERTY()
		FString ControllerName;

protected:
	virtual void Init(FString RosTopic);

	UPROPERTY()
		FString MessageType;
private:

	virtual void SetMessageType() {};
	virtual void CreateSubscriber() {};

};


