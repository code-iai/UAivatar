// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSCommandLiftArm.h"
#include "IAIAvatarROSIntegrationComponent.h"
#include "IAIAvatarCharacter.h"

#include "UIAIAvatarModule.h"


// Sets default values for this component's properties
UIAIAvatarROSCommandLiftArm::UIAIAvatarROSCommandLiftArm()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIAIAvatarROSCommandLiftArm::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UIAIAvatarROSCommandLiftArm::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIAIAvatarROSCommandLiftArm::OnROSSendCommandRequest(FString command) {
	UE_LOG(LogAvatarROS, Log, TEXT("OnROSSendCommandRequest received in UIAIAvatarROSCommandLiftArm"));

	if (command.Compare(FString("lift_arm")) != 0) {
		UE_LOG(LogAvatarROS, Warning, TEXT(" UIAIAvatarROSCommandLiftArm received non-responsible command: %s"), *command);
		return;
	}
		

	AActor* Owner = GetOwner();
	AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
	if (Avatar) {
		Avatar->StartRaiseHand();
	}

}