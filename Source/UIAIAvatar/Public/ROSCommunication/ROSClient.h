// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSrvClient.h"
#include "ROSClient.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UROSClient : public UObject
{
	GENERATED_BODY()
public:
	virtual void CallService() {};

	virtual void Init(UObject* InModel, TSharedPtr<FROSBridgeHandler> InHandler, FString InName);
	virtual void Init(UObject* InModel, TArray<FString>* OutArray, TSharedPtr<FROSBridgeHandler> InHandler);

	virtual void Init(UObject* InModel) {};
	virtual void Init(UObject* InModel, TArray<FString>* OutArray) {};
protected:

	TSharedPtr<FROSBridgeHandler> ROSHandler;

};
