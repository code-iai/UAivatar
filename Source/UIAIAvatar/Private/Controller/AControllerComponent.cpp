// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "Controller/AControllerComponent.h"

// Sets default values for this component's properties
UAControllerComponent::UAControllerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Agent = Cast<AActor>(GetOwner());
}

UAControllerComponent::~UAControllerComponent()
{
	UE_LOG(LogTemp, Error, TEXT("Destroy Controller Component"));
}

// Called when the game starts
void UAControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Agent)
	{
		UE_LOG(LogTemp, Error, TEXT("There is not an Agent as owner."));
	}
	else
	{
		for (auto& C : Controller.ControllerList)
		{
			C.Value->Init(Agent);
		}
	}
}

void UAControllerComponent::ExcecuteCommands()
{
	FString Command;
	while (!CommandQuerry.IsEmpty())
	{
		CommandQuerry.Dequeue(Command);
		ExcecuteCommand(Command);
	}
}

void UAControllerComponent::ExcecuteCommands(TArray<FString> InCommands)
{
	for (auto& Command : InCommands)
	{
		ExcecuteCommand(Command);
	}

}

void UAControllerComponent::ExcecuteCommand(FString InCommand)
{

}

UAController* UAControllerComponent::ControllerList(FString ControllerName)
{
	if (Controller.ControllerList.Contains(ControllerName))
	{
		return Controller.ControllerList[ControllerName];
	}
	return nullptr;
}

// Called every frame
void UAControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	float realtimeSeconds = FPlatformTime::Seconds();
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (auto& C : Controller.ControllerList)
	{
		C.Value->Tick(DeltaTime);
	}

	ExcecuteCommands();
}
