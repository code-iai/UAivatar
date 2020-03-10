// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "IAIAvatarAnimationInstance.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct FFingerRots_t
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator thumb_01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator index_01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator middle_01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator ring_01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator pinky_01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator thumb_02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator index_02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator middle_02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator ring_02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator pinky_02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator thumb_03;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator index_03;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator middle_03;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator ring_03;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator pinky_03;
};

UCLASS()
class UIAIAVATAR_API UIAIAvatarAnimationInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	// Target coordinates for the left hand of the Avatar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	FVector LeftHandIKTargetPosition;

	// Target coordinates for the right hand of the Avatar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	FVector RightHandIKTargetPosition;

	// Target coordinates for the left index finger of the Avatar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	FVector LeftFingerIKTargetPosition;

	// Target coordinates for the right index finger of the Avatar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	FVector RightFingerIKTargetPosition;

	// Additive Rotation on the left hand joint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	FRotator HandRotation;

	// Additive Rotation on the right hand joint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	FRotator RightHandRotation;

	// Additive Rotation on the spine joint # 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator Spine1Rotation;
	
	// Additive Rotation on the spine joint # 2
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator Spine2Rotation;

	// Additive Rotation on the hip joint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator HipRotation;

	// Additive Rotation on the head joint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator HeadRotation;

	// Additive Rotation on the neck joint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator NeckRotation;

	// A value between 0 and 1 controls the amount of blending between the animation and the inverse kinematic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float LeftHandIKAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the inverse kinematic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float RightHandIKAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the inverse kinematic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float LeftFingerIKAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the inverse kinematic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float RightFingerIKAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the bone modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float HandRotationAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the bone modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float RightHandRotationAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the bone modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float SpineRotationAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the bone modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float HeadRotationAlpha;

	// A value between 0 and 1 controls the amount of blending between the animation and the bone modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float NeckRotationAlpha;

	// True when object on left hand is grasped or false if nothing grasped. This is for proper animation and the bone modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float leftHandGraspingAlpha;

	// True when object on right hand is grasped or false if nothing grasped. This is for proper animation and the bone modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
	float rightHandGraspingAlpha;

	// All fingers rotations for grasping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FFingerRots_t RightHandFingerRots;

	// All fingers rotations for grasping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FFingerRots_t LeftHandFingerRots;

	// Rotate Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		float DesiredActorRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		bool EnableActorRotation;

	/** Left Lower Leg Offset From Ground, Set in Character.cpp Tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		FRotator SkelControl_Head;

	void resetRightHandFingerRots();
	void resetLeftHandFingerRots();
}; 