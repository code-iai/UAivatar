// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "UIAIAvatarModule.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"


/**
 * 
 */
class UIAIAVATAR_API InterpolationHandler : public UObject
{
public:
	InterpolationHandler() {

	}
	

	FString BoneName;

	FTimeline Timeline;

	UFUNCTION()
		void HandleTimelineProgress(float Value);

	UPROPERTY(EditAnywhere)
		UCurveFloat* CurveFloat;

	UPROPERTY(EditAnywhere)
		FRotator TargetRotation;

	FRotator InitialRotation;




	void Start() {
		if (CurveFloat)
		{
			FOnTimelineFloat ProgressFunction;
			ProgressFunction.BindUFunction(this, FName("HandleActorRotationTimelineProgress"));

			Timeline.AddInterpFloat(CurveFloat, ProgressFunction);
			Timeline.SetLooping(false);

			Timeline.PlayFromStart();
		}
		else
		{
			UE_LOG(LogAvatarCharacter, Error, TEXT("Can't execute SetAbsoluteActorRotationWithTimeline with an ActorRotationCurveFloat. Please set a curve in the Avatar Settings."));
		}
	}
};
