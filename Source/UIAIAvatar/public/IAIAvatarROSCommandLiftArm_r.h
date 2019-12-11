// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IAIAvatarROSCommandBase.h"
#include "IAIAvatarROSCommandLiftArm_r.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UIAIAVATAR_API UIAIAvatarROSCommandLiftArm_r : public UIAIAvatarROSCommandBase
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UIAIAvatarROSCommandLiftArm_r();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnROSSendCommandRequest(FString command) override;

};
