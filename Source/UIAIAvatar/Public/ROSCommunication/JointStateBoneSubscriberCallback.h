// Copyright 2022, Institute for Artificial Intelligence - University of Bremen
// Author: Mona Abdel-Keream (abdelker@uni-bremen.de)

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSubscriber.h"
#include "Controller/AvatarConsoleCommandController.h"

/**
 * 
 */
class UIAIAVATAR_API FJointStateBoneSubscriberCallback :public FROSBridgeSubscriber
{
public:
	FJointStateBoneSubscriberCallback(const FString& InTopic, const FString& InType, UObject* InController);
	~FJointStateBoneSubscriberCallback() override;

	TSharedPtr<FROSBridgeMsg> ParseMessage(TSharedPtr<FJsonObject> JsonObject) const override;

	void Callback(TSharedPtr<FROSBridgeMsg> Msg) override;

	UAvatarConsoleCommandController* Controller;

	//UFUNCTION(blueprintCallable, meta = (DisplayName = "UpdateBoneRotation"), Category = "BoneManipulation")
	//	static void UpdateBoneRotation(FRotator& BoneToRotate);	
	//UFUNCTION(blueprintCallable, meta = (DisplayName = "SetConstraintPosition"), Category = "constraints")
	//	static void SetConstraintPos(USkeletalMeshComponent* targetMesh, FName constraintName, FVector newPos);
	
};
