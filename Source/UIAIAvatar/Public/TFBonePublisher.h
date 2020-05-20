// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "IAIAvatarCharacter.h"
#include "GameFramework/Character.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgePublisher.h"
#include "tf2_msgs/TFMessage.h"
#include "geometry_msgs/TransformStamped.h"
#include "CoreMinimal.h"
#include "Core.h"
#include "Components/ActorComponent.h"
#include "TFBonePublisher.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )

class UIAIAVATAR_API UTFBonePublisher : public UActorComponent
{
GENERATED_BODY()

public:
// Sets default values for this component's properties
UTFBonePublisher();

protected:
// Called when the game starts
virtual void BeginPlay() override;

geometry_msgs::TransformStamped GetTransformStampedMsg(const FROSTime& InTime, const uint32 InSeq = 0, const uint32 bone_index = 0) const;
geometry_msgs::TransformStamped GetTransformStampedMsgRoot(const FROSTime& InTime, const uint32 InSeq = 0) const;


public:
// Called every frame
virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



// TF root frame name (map, world etc.)
UPROPERTY(EditAnywhere, Category = TF)
FString TFRootFrameName;

// Delta time (s) between publishing (0 = on Tick)
UPROPERTY(EditAnywhere, Category = TF, meta = (editcondition = "bUseConstantPublishRate", ClampMin = "0.0"))
float ConstantPublishRate;

float CurrentDeltaForPublishRate;


private:


// ROSBridge handler for ROS connection
TSharedPtr<FROSBridgeHandler> ROSBridgeHandler;

// ROSPublisher for publishing TF
TSharedPtr<FROSBridgePublisher> TFPublisher;

// Publisher timer handle (in case of custom publish rate)
FTimerHandle TFPubTimer;


// TF header message sequence
uint32 Seq;

};
