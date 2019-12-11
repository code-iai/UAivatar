// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
 #include "ROSBridgePublisher.h"
 #include "IAIAvatarCharacter.h"
#include "UIAIAvatarModule.h"

#include "IAIAvatarROSIntegrationComponent.generated.h"

/*
This Component provides ROS integration capabilities for a given Actor.
Please be aware that some features might require that the owning Actor is AI controlled!
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnROSSendCommandRequest, FString, command);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIAIAVATAR_API UIAIAvatarROSIntegrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIAIAvatarROSIntegrationComponent();

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "EventDispatchers")
	FOnROSSendCommandRequest ROSSendCommandRequest;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TSharedPtr<FROSBridgePublisher> Publisher;

  bool initialization_triggered = false;

  FTimerHandle TimerHandleROSInitialization;

public:

	// Set the Prefix for all ROS-related Namings. 
	// This includes the name for announced services and advertised topics
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	FString RosNamingPrefix;

	// Set to true to deactivate this component. This will cause BeginPlay() to skip any initialization and TickComponent returns immediately.
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateAllFunctionality;

	// Set to true to skip move_head service initialization
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateMoveHeadService;

	// Set to true to skip move_to service initialization
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateMoveToService;

	// Set to true to skip send_command service initialization
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateCommandService;

	// Set to true to skip send_console_command service initialization
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateConsoleCommandService;

	// Set to true to skip general grasp service initialization
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateGeneralGraspService;

	// Set to true to skip set_rotation service initialization
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateSetRotationService;

	// Set to true to skip /body_part_manipulation/set_interpolation_target service initialization
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateSetBodyPartInterpolation;

	// Set to true to skip /body_part_manipulation/start_enablement service initialization
	// This will usually gradually increase the alpha values of transform bone components
	// or IKs
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateStartBodyPartEnablement;

	// Set to true to skip /body_part_manipulation/start_disablement service initialization
	// This will usually gradually decrease the alpha values of transform bone components
	// or IKs
	UPROPERTY(EditAnywhere, Category = "ROS Integration Options")
	bool DeactivateStartBodyPartDisablement;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TriggerROSSendCommandRequest(FString command) {
		UE_LOG(LogAvatarROS, Log, TEXT("Received Trigger for ROSSendCommandRequest"));

		//Broadcasting the actual Event to all interested parties
		ROSSendCommandRequest.Broadcast(command);
	}

  // TODO decouple the avatar console command stuff from this class
	void TriggerROSSendConsoleCommandRequest(FString command) {
		UE_LOG(LogAvatarROS, Log, TEXT("Received Trigger for ROSSendConsoleCommandRequest"));

    AIAIAvatarCharacter* avatar = Cast<AIAIAvatarCharacter>(GetOwner());
    if(!avatar)
    {
		  UE_LOG(LogAvatarROS, Error, TEXT("Can't cast GetOwner() to AIAIAvatarCharacter in TriggerROSSendConsoleCommandRequest"));
      return;
    }
    avatar->ProcessConsoleCommand(command);
	}

  UFUNCTION()
  bool ConnectAndSetupROS();

  UFUNCTION()
  void TryToConnectAndSetupROS();
};
