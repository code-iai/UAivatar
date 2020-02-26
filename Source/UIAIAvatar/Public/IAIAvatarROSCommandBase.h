// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IAIAvatarROSCommandBase.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIAIAVATAR_API UIAIAvatarROSCommandBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIAIAvatarROSCommandBase();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// Callback Function that is called by the Delegate in IAIAvatarROSIntegrationComponent whenever a 
	// new Command has been received via the send_command service.
	//
	// Override this function to implement your individual behavior in your subclass.
	// Usually you do a comparison of @param command with your 
	// implemented commands. If @param command doesn't match, return immediately.
	// Else, execute the intended behavior.
	UFUNCTION()
	virtual void OnROSSendCommandRequest(FString command);
	
};
