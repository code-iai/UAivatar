// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSCommandCut.h"

#include "IAIAvatarROSIntegrationComponent.h"
#include "IAIAvatarCharacter.h"

#include "UIAIAvatarModule.h"


// Sets default values for this component's properties
UIAIAvatarROSCommandCut::UIAIAvatarROSCommandCut()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIAIAvatarROSCommandCut::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UIAIAvatarROSCommandCut::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIAIAvatarROSCommandCut::OnROSSendCommandRequest(FString command) {
	UE_LOG(LogAvatarROS, Log, TEXT("OnROSSendCommandRequest received in UIAIAvatarROSCommandCut"));

	if (command.Compare(FString("cut")) != 0) {
		UE_LOG(LogAvatarROS, Warning, TEXT(" UIAIAvatarROSCommandCut received non-responsible command: %s"), *command);
		return;
	}


	AActor* Owner = GetOwner();
	AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
	if (Avatar) {
		Avatar->ProcessConsoleCommand("cut");
	}

}


