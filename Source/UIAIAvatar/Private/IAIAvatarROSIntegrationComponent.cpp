// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSIntegrationComponent.h"
#include "ROSBridgeGameInstance.h"
#include "std_msgs/String.h"
#include "ROSMoveToServer.h"
#include "ROSAvatarRotationServer.h"
#include "ROSSendCommandServer.h"
#include "ROSSendConsoleCommandServer.h"
#include "ROSGeneralGraspServer.h"
#include "ROSSetBodyPartInterpolationServer.h"
#include "ROSStartBodyPartEnablementServer.h"
#include "ROSStartBodyPartDisablementServer.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "UIAIAvatarModule.h" // include for log levels

// Sets default values for this component's properties
UIAIAvatarROSIntegrationComponent::UIAIAvatarROSIntegrationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	DeactivateAllFunctionality = false;
	DeactivateMoveHeadService  = false;
}


// Called when the game starts
void UIAIAvatarROSIntegrationComponent::BeginPlay()
{
	Super::BeginPlay();

	  if (DeactivateAllFunctionality)
		return;

    FTimerDynamicDelegate eventTest;
    eventTest.BindDynamic(this, &UIAIAvatarROSIntegrationComponent::TryToConnectAndSetupROS);
    GetWorld()->GetTimerManager().SetTimer(TimerHandleROSInitialization, eventTest, 2.0, true);
}

void UIAIAvatarROSIntegrationComponent::TryToConnectAndSetupROS()
{
  UE_LOG(LogTemp, Log, TEXT("[%s] Try to set up ROS after timer in %s"), *FString(__FUNCTION__), *GetName());	
  if(ConnectAndSetupROS())
  {
    GetWorld()->GetTimerManager().ClearTimer(TimerHandleROSInitialization);
    return;
  }
  else
  {
    UE_LOG(LogTemp, Log, TEXT("[%s] ROS setup failed. Retrying..."), *FString(__FUNCTION__), *GetName());	
  }
}

bool UIAIAvatarROSIntegrationComponent::ConnectAndSetupROS()
{
  UE_LOG(LogTemp, Log, TEXT("[%s] Setting up ROS in UIAIAvatarROSIntegrationComponent."), *FString(__FUNCTION__));	

  // Accessing the Game Instance and checking for nullptr
  check(GetOwner());
  check(GetOwner()->GetGameInstance());

  // Getting the actual UROSBridgeGameInstance 
  UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetOwner()->GetGameInstance());
  check(ActiveGameInstance);

  int time_waited_ms = 0;
  int retry_time_ms = 50;

  // while(time_waited_ms <= 5000)
  // {
  if (!ActiveGameInstance->ROSHandler.IsValid())
  {
    UE_LOG(LogAvatarROS, Error, TEXT("Not connected to ROS. Invalid ROSHandler in IAIAvatarROSIntegrationComponent on %s"), *(GetOwner()->GetName()) );
    return false;
  }

  if (!ActiveGameInstance->ROSHandler->IsClientConnected())
  {
    UE_LOG(LogAvatarROS, Error, TEXT("ROSBridge client is not connected to ROS. Can't initialize IAIAvatarROSIntegrationComponent on %s"), *(GetOwner()->GetName()) );
    return false;
  }
  //-----------------------
  // Advertise Services
  //----------------------

  // /move_to
  if (!DeactivateMoveToService) {
    TSharedPtr<FROSMoveToServer> MoveToServiceServer =
      MakeShareable<FROSMoveToServer>(new FROSMoveToServer(this->GetOwner(), *(RosNamingPrefix + FString("/move_to")), TEXT("iai_avatar_msgs/MoveTo")));
    ActiveGameInstance->ROSHandler->AddServiceServer(MoveToServiceServer);
  }

  // /send_command
  if (!DeactivateCommandService) {
    TSharedPtr<FROSSendCommandServer> SendCommandServer =
      MakeShareable<FROSSendCommandServer>(new FROSSendCommandServer(this->GetOwner(), *(RosNamingPrefix + FString("/send_command")), TEXT("iai_avatar_msgs/Command")));
    ActiveGameInstance->ROSHandler->AddServiceServer(SendCommandServer);
  }

  // /send_console_command
  if (!DeactivateConsoleCommandService) {
    TSharedPtr<FROSSendConsoleCommandServer> SendConsoleCommandServer =
      MakeShareable<FROSSendConsoleCommandServer>(new FROSSendConsoleCommandServer(this->GetOwner(), *(RosNamingPrefix + FString("/send_console_command")), TEXT("iai_avatar_msgs/Command")));
    ActiveGameInstance->ROSHandler->AddServiceServer(SendConsoleCommandServer);
  }

  // /GeneralGrasp
  if (!DeactivateCommandService) {
    TSharedPtr<FROSGeneralGraspServer> GeneralGraspServer =
      MakeShareable<FROSGeneralGraspServer>(new FROSGeneralGraspServer(this->GetOwner(), *(RosNamingPrefix + FString("/general_grasp")), TEXT("iai_avatar_msgs/GeneralGrasp")));
    ActiveGameInstance->ROSHandler->AddServiceServer(GeneralGraspServer);
  }

  // /set_rotation
  if (!DeactivateSetRotationService) {
    TSharedPtr<FROSAvatarRotationServer> AvatarRotationServer =
      MakeShareable<FROSAvatarRotationServer>(new FROSAvatarRotationServer(this->GetOwner(), *(RosNamingPrefix + FString("/set_rotation")), TEXT("iai_avatar_msgs/AvatarRotation")));
    ActiveGameInstance->ROSHandler->AddServiceServer(AvatarRotationServer);
  }

  // /body_part_manipulation/set_interpolation_target
  if (!DeactivateSetBodyPartInterpolation) {
    TSharedPtr<FROSSetBodyPartInterpolationServer> SetLeftHandIKServer =
      MakeShareable<FROSSetBodyPartInterpolationServer>(new FROSSetBodyPartInterpolationServer(this->GetOwner(), *(RosNamingPrefix + FString("/body_part_manipulation/set_interpolation_target")), TEXT("iai_avatar_msgs/NamedTargetInterpolation")));
    ActiveGameInstance->ROSHandler->AddServiceServer(SetLeftHandIKServer);
  }

  //  /body_part_manipulation/start_enablement
  if (!DeactivateStartBodyPartEnablement) {
    TSharedPtr<FROSStartBodyPartEnablementServer> StartBodyPartEnablementServer =
      MakeShareable<FROSStartBodyPartEnablementServer>(new FROSStartBodyPartEnablementServer(this->GetOwner(), *(RosNamingPrefix + FString("/body_part_manipulation/start_enablement")), TEXT("iai_avatar_msgs/NamedTargetInterpolation")));
    ActiveGameInstance->ROSHandler->AddServiceServer(StartBodyPartEnablementServer);
  }

  //  /body_part_manipulation/start_enablement
  if (!DeactivateStartBodyPartDisablement) {
    TSharedPtr<FROSStartBodyPartDisablementServer> StartBodyPartDisablementServer =
      MakeShareable<FROSStartBodyPartDisablementServer>(new FROSStartBodyPartDisablementServer(this->GetOwner(), *(RosNamingPrefix + FString("/body_part_manipulation/start_disablement")), TEXT("iai_avatar_msgs/NamedTargetInterpolation")));
    ActiveGameInstance->ROSHandler->AddServiceServer(StartBodyPartDisablementServer);
  }

  ActiveGameInstance->ROSHandler->Process();

  return true;
}

// Called every frame
void UIAIAvatarROSIntegrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeactivateAllFunctionality)
		return;
}

