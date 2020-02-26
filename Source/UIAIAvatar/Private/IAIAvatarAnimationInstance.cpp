// Fill out your copyright notice in the Description page of Project Settings.

#include "IAIAvatarAnimationInstance.h"

UIAIAvatarAnimationInstance::UIAIAvatarAnimationInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//set any default values for your variables here
	SkelControl_Head = FRotator(0, 0, 0);
}

void UIAIAvatarAnimationInstance::resetRightHandFingerRots() {
	
	FRotator zeroRots = FRotator(0,0,0);
	
	RightHandFingerRots.index_01  = zeroRots;
	RightHandFingerRots.index_02  = zeroRots;
	RightHandFingerRots.index_03  = zeroRots;
	RightHandFingerRots.middle_01 = zeroRots;
	RightHandFingerRots.middle_02 = zeroRots;
	RightHandFingerRots.middle_03 = zeroRots;
	RightHandFingerRots.ring_01   = zeroRots;
	RightHandFingerRots.ring_02   = zeroRots; 
	RightHandFingerRots.ring_03   = zeroRots;
	RightHandFingerRots.pinky_01  = zeroRots;
	RightHandFingerRots.pinky_02  = zeroRots;
	RightHandFingerRots.pinky_03  = zeroRots;
	RightHandFingerRots.thumb_01  = zeroRots;
	RightHandFingerRots.thumb_02  = zeroRots;
	RightHandFingerRots.thumb_03  = zeroRots;
}

void UIAIAvatarAnimationInstance::resetLeftHandFingerRots() {
	
	FRotator zeroRots = FRotator(0, 0, 0);
	LeftHandFingerRots.index_01  = zeroRots;
	LeftHandFingerRots.index_02  = zeroRots;
	LeftHandFingerRots.index_03  = zeroRots;
	LeftHandFingerRots.middle_01 = zeroRots;
	LeftHandFingerRots.middle_02 = zeroRots;
	LeftHandFingerRots.middle_03 = zeroRots;
	LeftHandFingerRots.ring_01   = zeroRots;
	LeftHandFingerRots.ring_02   = zeroRots;
	LeftHandFingerRots.ring_03   = zeroRots;
	LeftHandFingerRots.pinky_01  = zeroRots;
	LeftHandFingerRots.pinky_02  = zeroRots;
	LeftHandFingerRots.pinky_03  = zeroRots;
	LeftHandFingerRots.thumb_01  = zeroRots;
	LeftHandFingerRots.thumb_02  = zeroRots;
	LeftHandFingerRots.thumb_03  = zeroRots;
}

