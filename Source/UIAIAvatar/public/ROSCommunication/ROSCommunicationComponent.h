// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ROSCommunication/ROSCommunicationContainer.h"
#include "ROSCommunicationComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UIAIAVATAR_API UROSCommunicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UROSCommunicationComponent();

	UPROPERTY(EditAnywhere)
		FROSCommunicationContainer RosCommunication;

	UPROPERTY(EditAnywhere)
		bool turnOffRosCommunication;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};