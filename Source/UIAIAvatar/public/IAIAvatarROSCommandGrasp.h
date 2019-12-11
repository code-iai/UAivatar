// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IAIAvatarROSCommandBase.h"
#include "IAIAvatarROSCommandGrasp.generated.h"

/*
  This class handles grasping related ROS Commands for example the actual Grasping Motion of an Object or the detaching from it.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIAIAVATAR_API UIAIAvatarROSCommandGrasp : public UIAIAvatarROSCommandBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIAIAvatarROSCommandGrasp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnROSSendCommandRequest(FString command) override;
	
};
