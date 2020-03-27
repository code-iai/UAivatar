// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSCommandSpoon.h"

#include "IAIAvatarROSIntegrationComponent.h"
#include "IAIAvatarCharacter.h"

#include "UIAIAvatarModule.h"


// Sets default values for this component's properties
UIAIAvatarROSCommandSpoon::UIAIAvatarROSCommandSpoon()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIAIAvatarROSCommandSpoon::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UIAIAvatarROSCommandSpoon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIAIAvatarROSCommandSpoon::OnROSSendCommandRequest(FString command) {
	UE_LOG(LogAvatarROS, Log, TEXT("OnROSSendCommandRequest received in UIAIAvatarROSCommandSpoon"));

	if (command.Compare(FString("spoon")) != 0) {
		UE_LOG(LogAvatarROS, Warning, TEXT(" UIAIAvatarROSCommandSpoon received non-responsible command: %s"), *command);
		return;
	}


	AActor* Owner = GetOwner();
	AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
	if (Avatar) {
		Avatar->ProcessConsoleCommand("spoon");
	}

}



