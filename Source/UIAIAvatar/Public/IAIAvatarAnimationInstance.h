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

	FFingerRots_t operator *=(float B) {
		index_01 *= B;
		index_02 *= B;
		index_03 *= B;
		pinky_01 *= B;
		pinky_02 *= B;
		pinky_03 *= B;
		ring_01 *= B;
		ring_02 *= B;
		ring_03 *= B;
		middle_01 *= B;
		middle_02 *= B;
		middle_03 *= B;
		thumb_01 *= B;
		thumb_02 *= B;
		thumb_03 *= B;
		return *this;
	};

	FFingerRots_t operator +=(const FFingerRots_t &B) {
		index_01 += B.index_01;
		index_02 += B.index_02;
		index_03 += B.index_03;
		pinky_01 += B.pinky_01;
		pinky_02 += B.pinky_02;
		pinky_03 += B.pinky_03;
		ring_01 += B.ring_01;
		ring_02 += B.ring_02;
		ring_03 += B.ring_03;
		middle_01 += B.middle_01;
		middle_02 += B.middle_02;
		middle_03 += B.middle_03;
		thumb_01 += B.thumb_01;
		thumb_02 += B.thumb_02;
		thumb_03 += B.thumb_03;
		return *this;
	};

	FFingerRots_t operator -=(const FFingerRots_t &B) {
		index_01 -= B.index_01;
		index_02 -= B.index_02;
		index_03 -= B.index_03;
		pinky_01 -= B.pinky_01;
		pinky_02 -= B.pinky_02;
		pinky_03 -= B.pinky_03;
		ring_01 -= B.ring_01;
		ring_02 -= B.ring_02;
		ring_03 -= B.ring_03;
		middle_01 -= B.middle_01;
		middle_02 -= B.middle_02;
		middle_03 -= B.middle_03;
		thumb_01 -= B.thumb_01;
		thumb_02 -= B.thumb_02;
		thumb_03 -= B.thumb_03;
		return *this;
	};

	FFingerRots_t operator -(const FFingerRots_t &B) {
		FFingerRots_t Res;
		Res.index_01 = index_01 - B.index_01;
		Res.index_02 = index_02 - B.index_02;
		Res.index_03 = index_03 - B.index_03;
		Res.pinky_01 = pinky_01 - B.pinky_01;
		Res.pinky_02 = pinky_02 - B.pinky_02;
		Res.pinky_03 = pinky_03 - B.pinky_03;
		Res.ring_01 = ring_01 - B.ring_01;
		Res.ring_02 = ring_02 - B.ring_02;
		Res.ring_03 = ring_03 - B.ring_03;
		Res.middle_01 = middle_01 - B.middle_01;
		Res.middle_02 = middle_02 - B.middle_02;
		Res.middle_03 = middle_03 - B.middle_03;
		Res.thumb_01 = thumb_01 - B.thumb_01;
		Res.thumb_02 = thumb_02 - B.thumb_02;
		Res.thumb_03 = thumb_03 - B.thumb_03;
		return Res;
	};

	FFingerRots_t operator +(const FFingerRots_t &B) {
		FFingerRots_t Res;
		Res.index_01 = index_01 + B.index_01;
		Res.index_02 = index_02 + B.index_02;
		Res.index_03 = index_03 + B.index_03;
		Res.pinky_01 = pinky_01 + B.pinky_01;
		Res.pinky_02 = pinky_02 + B.pinky_02;
		Res.pinky_03 = pinky_03 + B.pinky_03;
		Res.ring_01 = ring_01 + B.ring_01;
		Res.ring_02 = ring_02 + B.ring_02;
		Res.ring_03 = ring_03 + B.ring_03;
		Res.middle_01 = middle_01 + B.middle_01;
		Res.middle_02 = middle_02 + B.middle_02;
		Res.middle_03 = middle_03 + B.middle_03;
		Res.thumb_01 = thumb_01 + B.thumb_01;
		Res.thumb_02 = thumb_02 + B.thumb_02;
		Res.thumb_03 = thumb_03 + B.thumb_03;
		return Res;
	};

	FFingerRots_t operator *(float B) {
		FFingerRots_t Res;
		Res.index_01 = index_01 * B;
		Res.index_02 = index_02 * B;
		Res.index_03 = index_03 * B;
		Res.pinky_01 = pinky_01 * B;
		Res.pinky_02 = pinky_02 * B;
		Res.pinky_03 = pinky_03 * B;
		Res.ring_01 = ring_01 * B;
		Res.ring_02 = ring_02 * B;
		Res.ring_03 = ring_03 * B;
		Res.middle_01 = middle_01 * B;
		Res.middle_02 = middle_02 * B;
		Res.middle_03 = middle_03 * B;
		Res.thumb_01 = thumb_01 * B;
		Res.thumb_02 = thumb_02 * B;
		Res.thumb_03 = thumb_03 * B;
		return Res;
	};

	bool IsZero() {
		bool res = true;
		res &= index_01.IsZero();
		res &= index_02.IsZero();
		res &= index_03.IsZero();
		res &= pinky_01.IsZero();
		res &= pinky_02.IsZero();
		res &= pinky_03.IsZero();
		res &= ring_01.IsZero();
		res &= ring_02.IsZero();
		res &= ring_03.IsZero();
		res &= middle_01.IsZero();
		res &= middle_02.IsZero();
		res &= middle_03.IsZero();
		res &= thumb_01.IsZero();
		res &= thumb_02.IsZero();
		res &= thumb_03.IsZero();
		return res;
	};
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

    // Additive Rotation on the head joint
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
    FRotator JawRotation;

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
    float JawRotationAlpha;

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

	// Activate sitting animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		bool bActivateSitAnim;

	// Move to left sit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IAIAvatar)
		bool bActivateMoveToLeftSit;

	void resetRightHandFingerRots();
	void resetLeftHandFingerRots();
       UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator spine_02_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator spine_05_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator neck_01_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator neck_02_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator head_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator clavicle_l_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator clavicle_r_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator upperarm_l_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator upperarm_r_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator lowerarm_l_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator lowerarm_r_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator hand_l_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator hand_r_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator calf_l_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator calf_r_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator thigh_l_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator thigh_r_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator foot_l_Rotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IAIAvatar)
	FRotator foot_r_Rotation;
	
}; 
