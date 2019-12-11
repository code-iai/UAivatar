// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSCommandGrasp_r.h"
#include "IAIAvatarCharacter.h"
#include "UIAIAvatarModule.h"


// Sets default values for this component's properties
UIAIAvatarROSCommandGrasp_r::UIAIAvatarROSCommandGrasp_r()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIAIAvatarROSCommandGrasp_r::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UIAIAvatarROSCommandGrasp_r::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIAIAvatarROSCommandGrasp_r::OnROSSendCommandRequest(FString command) {
	UE_LOG(LogAvatarROS, Log, TEXT("OnROSSendCommandRequest received in UIAIAvatarROSCommandGrasp_r"));

	if (command.Compare(FString("start_grasp_r")) == 0) {
		AActor* Owner = GetOwner();
		AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
		if (Avatar) {
			Avatar->StartGraspObject_r();
		}

		return;
	}

	if (command.Compare(FString("stop_grasp_r")) == 0) {
		AActor* Owner = GetOwner();
		AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
		if (Avatar) {
			Avatar->StopGraspObject_r();
		}

		return;
	}

	if (command.Compare(FString("detach_object_r")) == 0) {
		AActor* Owner = GetOwner();
		AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
		if (Avatar) {
			Avatar->DetachGraspObject_r();
		}

		return;
	}
}