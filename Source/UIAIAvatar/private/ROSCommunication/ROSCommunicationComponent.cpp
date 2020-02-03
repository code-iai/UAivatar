// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunicationComponent.h"

// Sets default values for this component's properties
UROSCommunicationComponent::UROSCommunicationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}


// Called when the game starts
void UROSCommunicationComponent::BeginPlay()
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
void UROSCommunicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	RosComunication.Tick(DeltaTime);
}

void UROSCommunicationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RosComunication.DeInit();
	Super::EndPlay(EndPlayReason);
}


