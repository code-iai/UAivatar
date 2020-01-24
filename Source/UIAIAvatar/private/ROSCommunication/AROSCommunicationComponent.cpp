// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "AROSCommunicationComponent.h"

// Sets default values for this component's properties
UAROSCommunicationComponent::UAROSCommunicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}


// Called when the game starts
void UAROSCommunicationComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor * Owner = GetOwner();
	RosComunication.ControllerComponent = Owner->FindComponentByClass<UAControllerComponent>();
	if (RosComunication.ControllerComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Found Controller comp"));
		RosComunication.Init();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No FROSBridgeHandler created."));
	}

}


// Called every frame
void UAROSCommunicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	RosComunication.Tick(DeltaTime);
}

void UAROSCommunicationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RosComunication.DeInit();
	Super::EndPlay(EndPlayReason);
}


