// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSPublisher.h"
#include "Components/ActorComponent.h"
#include "Controller/AvatarQnAController.h"
#include "Controller/AControllerComponent.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgePublisher.h"
#include "std_msgs/Int32.h"
#include "BeliefStatePublisher.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIAIAVATAR_API UBeliefStatePublisher : public UActorComponent
{
	GENERATED_BODY()

public:
	UBeliefStatePublisher();

	//virtual void SetBelief(UObject* InOwner);

	//// Belief state for Shelf state
	UPROPERTY(EditAnywhere, Category = BeliefState)
		bool bShelfIsEmpty = false;

		bool GetValue();

		void SetValue();
	
protected:

	

	// Delta time (s) between publishing (0 = on Tick)


	UPROPERTY(EditAnywhere, Category = BeliefState, meta = (editcondition = "bUseConstantPublishRate", ClampMin = "0.0"))
		float ConstantPublishRate;

	float CurrentDeltaForPublishRate;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//virtual void Publish();

private:


	// ROSBridge handler for ROS connection
	TSharedPtr<FROSBridgeHandler> ROSBridgeHandler;

	// ROSPublisher for publishing TF
	TSharedPtr<FROSBridgePublisher> BFSPublisher;

	// Publisher timer handle (in case of custom publish rate)
	FTimerHandle BFSPubTimer;


	// TF header message sequence
	uint32 Seq;





};
