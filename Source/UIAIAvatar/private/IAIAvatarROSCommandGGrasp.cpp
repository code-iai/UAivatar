// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSCommandGGrasp.h"
#include "IAIAvatarCharacter.h"
#include "UIAIAvatarModule.h"

// Sets default values for this component's properties
UIAIAvatarROSCommandGGrasp::UIAIAvatarROSCommandGGrasp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIAIAvatarROSCommandGGrasp::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UIAIAvatarROSCommandGGrasp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIAIAvatarROSCommandGGrasp::OnROSSendCommandRequest(FString command) {
	UE_LOG(LogAvatarROS, Log, TEXT("OnROSSendCommandRequest received in UIAIAvatarROSCommandGrasp"));

	if (command.Compare(FString("start_grasp")) == 0) {
		AActor* Owner = GetOwner();
		AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
		if (Avatar) {
			Avatar->StartGraspObject();
		}

		return;
	}

	if (command.Compare(FString("stop_grasp")) == 0) {
		AActor* Owner = GetOwner();
		AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
		if (Avatar) {
			Avatar->StopGraspObject();
		}

		return;
	}

	if (command.Compare(FString("detach_object")) == 0) {
		AActor* Owner = GetOwner();
		AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
		if (Avatar) {
			Avatar->DetachGraspObject();
		}

		return;
	}
}