// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/ROSCommunicationComponent.h"

// Sets default values for this component's properties
UROSCommunicationComponent::UROSCommunicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	turnOffRosCommunication = false;
}


// Called when the game starts
void UROSCommunicationComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!turnOffRosCommunication) {
		AActor * Owner = GetOwner();
		RosCommunication.ControllerComponent = Owner->FindComponentByClass<UAControllerComponent>();
		if (RosCommunication.ControllerComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("Found Controller comp"));
			RosCommunication.Init();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No FROSBridgeHandler created."));
		}
	}
}


// Called every frame
void UROSCommunicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!turnOffRosCommunication)
		RosCommunication.Tick(DeltaTime);
}

void UROSCommunicationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RosCommunication.DeInit();
	Super::EndPlay(EndPlayReason);
}


