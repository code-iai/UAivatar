// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSCommandLiftArm_r.h"
#include "IAIAvatarROSIntegrationComponent.h"
#include "IAIAvatarCharacter.h"

#include "UIAIAvatarModule.h"


// Sets default values for this component's properties
UIAIAvatarROSCommandLiftArm_r::UIAIAvatarROSCommandLiftArm_r()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIAIAvatarROSCommandLiftArm_r::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UIAIAvatarROSCommandLiftArm_r::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIAIAvatarROSCommandLiftArm_r::OnROSSendCommandRequest(FString command) {
	UE_LOG(LogAvatarROS, Log, TEXT("OnROSSendCommandRequest received in UIAIAvatarROSCommandLiftArm_r"));

	if (command.Compare(FString("lift_arm_r")) != 0) {
		UE_LOG(LogAvatarROS, Warning, TEXT(" UIAIAvatarROSCommandLiftArm_r received non-responsible command: %s"), *command);
		return;
	}


	AActor* Owner = GetOwner();
	AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
	if (Avatar) {
		Avatar->StartRaiseHand_r();
	}

}