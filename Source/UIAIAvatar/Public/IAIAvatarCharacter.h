// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "IAIAvatarAnimationInstance.h"
//#include "PIDController3D.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "InterpolationHandler.h"
#include "ProceduralMeshHelper.h"
#include "IAIAvatarCharacter.generated.h"

UCLASS(config=Game)
class AIAIAvatarCharacter : public ACharacter
{
	GENERATED_BODY()

		// ************************** Private Parameters *******************************

		/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	// **************************** Private Methods ********************************

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	// **************************** Protected Parameters ***************************
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	// ***************************** Protected Methods *****************************

	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	void TurnCam(float Axis);

	void LookCam(float Axis);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/** Resets HMD orientation in VR. */
	void OnResetVR();

public:

	// *************************** Public Parameters *******************************

	// Interpolation data structures for different Types

	struct InterpolationContainer {
		FOnTimelineFloat ProgressFunction;
		FTimeline Timeline;
	};

	struct FloatInterpolationContainer : InterpolationContainer {
		float InitialValue;
		float TargetValue;
	};

	struct RotatorInterpolationContainer : InterpolationContainer {
		FRotator InitialValue;
		FRotator TargetValue;
	};

	struct VectorInterpolationContainer : InterpolationContainer {
		FVector InitialValue;
		FVector TargetValue;
	};

	// ********** Target Poses **********

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		ATargetPoint *TargetPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FVector HandTargetPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FVector RightHandTargetPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FVector FingerTargetPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FVector RightFingerTargetPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FRotator Spine1Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FRotator Spine2Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FRotator HipRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FRotator HandRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		FRotator RightHandRotation;

	// ********** For Grasping **********

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		AActor* ObjectToGrasp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		AActor* ObjectToGrasp_r;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		bool isGrasped_r = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		bool isGrasped_l = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		bool isGrasping_r = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		bool isGrasping_l = false;

	// Current object on right hand
	AActor* graspedObject_r;

	// Current object on left hand
	AActor* graspedObject_l;

	// ******** For Hands/Fingers/Spine/Actor IK & Rots ********

	// Actor Rotation 
	FTimeline ActorRotationTimeline;
	FRotator InitialActorRotation;
	TSharedPtr<InterpolationHandler> RotationHandler;

	// Actor Rotation curve. Control the acceleration curve between the Target Rotation and the current Rotation.
	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		UCurveFloat* ActorRotationCurveFloat;

	// Where the Actor should rotate to
	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		FRotator ActorRotationTarget;

	// Spine Manipulation
	FloatInterpolationContainer SpineInterpolation;
	RotatorInterpolationContainer SpineBoneInterpolation;

	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		UCurveFloat* SpineCurveFloat;

	// Lefthandrotation Manipulation
	FloatInterpolationContainer LeftHandRotationAlphaInterpolation;
	RotatorInterpolationContainer LeftHandRotationBoneInterpolation;

	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		UCurveFloat* HandRotationCurveFloat;

	// Righthandrotation Manipulation
	FloatInterpolationContainer RightHandRotationAlphaInterpolation;
	RotatorInterpolationContainer RightHandRotationBoneInterpolation;

	// Lefthand IK 
	FloatInterpolationContainer LeftHandIKAlphaInterpolation;
	VectorInterpolationContainer LeftHandIKBoneInterpolation;

	// Curve. Control the acceleration curve between the Target Rotation and the current Rotation.
	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		UCurveFloat* LeftHandIKCurveFloat;

	// Righthand IK 
	FloatInterpolationContainer RightHandIKAlphaInterpolation;
	VectorInterpolationContainer RightHandIKBoneInterpolation;

	// Curve. Control the acceleration curve between the Target Rotation and the current Rotation.
	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		UCurveFloat* RightHandIKCurveFloat;

	// LeftFinger IK 
	FloatInterpolationContainer LeftFingerIKAlphaInterpolation;
	VectorInterpolationContainer LeftFingerIKBoneInterpolation;

	// Curve. Control the acceleration curve between the Target Rotation and the current Rotation.
	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		UCurveFloat* LeftFingerIKCurveFloat;

	// RightFinger IK 
	FloatInterpolationContainer RightFingerIKAlphaInterpolation;
	VectorInterpolationContainer RightFingerIKBoneInterpolation;

	// Curve. Control the acceleration curve between the Target Rotation and the current Rotation.
	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Rotation")
		UCurveFloat* RightFingerIKCurveFloat;

	// ******** For Head ********

	// Head Controller
//	UPROPERTY(EditAnywhere, Category = "IAI Avatar Configuration|Head PID Controller")
//		FPIDController3D HeadPIDController;

	// ******** For IK Handlers ********

	// Smooth IK blending variables
	float IKEnableTimer = 0;
	float IKEnableTickDirection = 1.0f;

	// Right smooth IK blending variables
	float IKEnableTimer_r = 0;
	float IKEnableTickDirection_r = 1.0f;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IKEnableActive = false; // True if smooth IK enabling shall be performed

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IKEnableActive_r = false; // True if smooth IK enabling shall be performed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		float InterpSpeed = 4.0f; // In seconds

	// ******** For Others ********

	// Animation Instances
	UIAIAvatarAnimationInstance* CurrentAnimationInstance;

	TMap<FString, FHitResult> UniqueHits;

	// Current object targeted for grasping
	TPair<int32, FString> current_obj_target;

	// For enabling Head Movement Recognition 
	bool enableDTW;

	// ************************** Public Methods ***********************************

	/* Constructor */
	AIAIAvatarCharacter();

	// ******** Hands/Fingers/Spine/Actor IK & Rots ********

	// Actor Rotation
	UFUNCTION()
		void HandleActorRotationTimelineProgress(float Value); // The function that will handle every tick of the float curve

	void SetAbsoluteActorRotationWithTimeline(FRotator rot);

	// Spine Rotation
	UFUNCTION()
		void HandleSpineAlphaInterpolationProgress(float Value);
	UFUNCTION()
		void HandleSpineBoneInterpolationProgress(float Value);

	void StartSpineEnablement(FRotator Spine1, FRotator Spine2 = FRotator(0, 0, 0), FRotator Hip = FRotator(0, 0, 0));	// Start Alpha Enablement when coming from an animation
	void InterpolateSpineTo(FRotator NewRot); // Interpolate to NewRot, given that Alpha on the corresponding modify bone is 1
	void StartSpineDisablement();	// Start Alpha Disablement. Decrease Alpha of modify bone slowly to 0

	// Left Hand rotation
	UFUNCTION()
		void HandleLeftHandRotationAlphaInterpolationProgress(float Value);
	UFUNCTION()
		void HandleLeftHandRotationBoneInterpolationProgress(float Value);

	void StartLeftHandRotationEnablement(FRotator NewRot);	// Start Alpha Enablement when coming from an animation
	void InterpolateLeftHandRotationTo(FRotator NewRot); // Interpolate to NewRot, given that Alpha on the corresponding modify bone is 1
	void StartLeftHandRotationDisablement();	// Start Alpha Disablement. Decrease Alpha of modify bone slowly to 0

	// Right Hand Rotation
	UFUNCTION()
		void HandleRightHandRotationAlphaInterpolationProgress(float Value);
	UFUNCTION()
		void HandleRightHandRotationBoneInterpolationProgress(float Value);

	void StartRightHandRotationEnablement(FRotator NewRot);	// Start Alpha Enablement when coming from an animation
	void InterpolateRightHandRotationTo(FRotator NewRot); // Interpolate to NewRot, given that Alpha on the corresponding modify bone is 1
	void StartRightHandRotationDisablement();	// Start Alpha Disablement. Decrease Alpha of modify bone slowly to 0

	// Left Hand IK
	UFUNCTION()
		void HandleLeftHandIKAlphaInterpolationProgress(float Value);
	UFUNCTION()
		void HandleLeftHandIKBoneInterpolationProgress(float Value);

	void StartLeftHandIKEnablement(FVector NewVec);	// Start Alpha Enablement when coming from an animation
	void InterpolateLeftHandIKTo(FVector NewVec); // Interpolate to NewRot, given that Alpha on the corresponding modify bone is 1
	void StartLeftHandIKDisablement();	// Start Alpha Disablement. Decrease Alpha of modify bone slowly to 0

	// Right Hand IK
	UFUNCTION()
		void HandleRightHandIKAlphaInterpolationProgress(float Value);
	UFUNCTION()
		void HandleRightHandIKBoneInterpolationProgress(float Value);

	void StartRightHandIKEnablement(FVector NewVec);	// Start Alpha Enablement when coming from an animation
	void InterpolateRightHandIKTo(FVector NewVec); // Interpolate to NewRot, given that Alpha on the corresponding modify bone is 1
	void StartRightHandIKDisablement();	// Start Alpha Disablement. Decrease Alpha of modify bone slowly to 0

	// Left Index IK
	UFUNCTION()
		void HandleLeftFingerIKAlphaInterpolationProgress(float Value);
	UFUNCTION()
		void HandleLeftFingerIKBoneInterpolationProgress(float Value);

	void StartLeftFingerIKEnablement(FVector NewVec);	// Start Alpha Enablement when coming from an animation
	void InterpolateLeftFingerIKTo(FVector NewVec); // Interpolate to NewRot, given that Alpha on the corresponding modify bone is 1
	void StartLeftFingerIKDisablement();	// Start Alpha Disablement. Decrease Alpha of modify bone slowly to 0

	// Right Index IK
	UFUNCTION()
		void HandleRightFingerIKAlphaInterpolationProgress(float Value);
	UFUNCTION()
		void HandleRightFingerIKBoneInterpolationProgress(float Value);

	void StartRightFingerIKEnablement(FVector NewVec);	// Start Alpha Enablement when coming from an animation
	void InterpolateRightFingerIKTo(FVector NewVec); // Interpolate to NewRot, given that Alpha on the corresponding modify bone is 1
	void StartRightFingerIKDisablement();	// Start Alpha Disablement. Decrease Alpha of modify bone slowly to 0

	// ******** IK Handlers ********

	// Handle the smooth increasing/decreasing of the IK blending alpha
	// Will usually be called in Tick()
	void HandleIKEnablement_l(float DeltaTime);

	// Handle the right smooth increasing/decreasing of the IK blending alpha
	// Will usually be called in Tick()
	void HandleIKEnablement_r(float DeltaTime);

	// Reconfigure the IK parameters if the relevant variables have been changed in the editor
	void PostEditChangeProperty(struct FPropertyChangedEvent& e);

	// ******** Grasping ********

	/* Attach function */
	UFUNCTION(BlueprintCallable)
		void AttachObjectCPP();

	/**TurnHand Start Left Grasp Handler. Modifies the l_hand bone */
	void StartGraspObject();

	/**TurnHand Start Right Grasp Handler. Modifies the r_hand bone */
	void StartGraspObject_r();

	/**TurnHand Stop Left Grasp Handler. Aborts grasping if called. Modifies the l_hand bone*/
	void StopGraspObject();

	/**TurnHand Stop Right Grasp Handler. Aborts grasping if called. Modifies the r_hand bone*/
	void StopGraspObject_r();

	/** Set a target object to grasp*/
	void SetTargetObject();

	// Function for general grasping
	void StartGrasp(FHitResult object, bool hold = false);

	/** Run StartGrasp for a target object */
	void GraspTargetObject();

	/** Run StartGrasp for a target object */
	TArray<FString> GraspTargetObject_ROS(FString targetObject, bool hold = false);

	/** Place object on left hand over/in specified place */
	void PlaceObject(FString targetPlace, FString Hand = "any", FVector targetPoint = FVector(0, 0, 0));

	/**Detach Left Object after Grasping*/
	void DetachGraspObject();
	void DetachGraspedObject_l();

	/**Detach Right Object after Grasping*/
	void DetachGraspObject_r();
	void DetachGraspedObject_r();

	// ******** Raising hands ********

	/** Start Left Hand raising */
	void StartRaiseHand();

	/** Start Right Hand raising */
	void StartRaiseHand_r();

	/** Stop Left Hand raising */
	void StopRaiseHand();

	/** Stop Right Hand raising */
	void StopRaiseHand_r();

	// ******** Reaching ********

	// Reach for an Actor. Modifies the l_hand bone
	void LeftHandReachForActor(AActor* Actor);

	// Reach to specific coordinates. Modifies the l_hand bone
	void StartLeftHandReach(FVector Target);

	// Reach to specific coordinates. Modifies the r_hand bone
	void StartRightHandReach(FVector Target);

	// Stop left hand reaching motion
	void StopLeftHandReach();

	// Stop right hand reaching motion
	void StopRightHandReach();

	// Stop hands reaching motion
	UFUNCTION(BlueprintCallable)
		void StopHandReach();

	// ******** Moving Avatar ********

	// Navigate to this->TargetPoint by using SimpleMoveToActor. Usable on Player and AI controlled Actors.
	void SimpleMoveToTargetPoint();

	/* Navigate to a certain goal with the AI system. Does only work with AI controlled Actors because
	the precise version of MoveToLocation is only realized for AI controlled Actors.

	@pre ControlledByAI() == true
	*/
	void MoveTo(FVector PositionInWorld);

	void TurnTo(FString mode, float angle = 0);

	void StartPathFollowing(FString Path);
	//void FollowSpline();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
		bool isSplineRunning = false;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
	//	USplineComponent* TrackRef;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IAI Avatar Configuration")
	//	AActor* LevelSpline;

	// ******** Other Functions ********

	// Check if this Character is currently controlled by an AAIController
	bool ControlledByAI();

	// Check if this Character is currently controlled by an APlayerController
	bool ControlledByPlayer();

	/* Process commmands from console */
	UFUNCTION(BlueprintCallable)
		void ProcessConsoleCommand(FString inLine);

	void Feed(FString Character);

	/*C++ empty event (BP implemented) to be called from C++*/
	/* Uses BP's multi trace by channel */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		TArray<FHitResult> TraceObjectsWithBP();

	// List reachable objects 
	UFUNCTION(BlueprintCallable)
	TMap<FString, FHitResult> ListObjects();

	/** Press microwave button */
	void PressMicrowaveButton(FString button);

	/** Close Door */
	void CloseDoor(FString door);

	void HighLightObject(FString Object_Name, bool OnOff);

	/** Reset Camera Rotation*/
	void ResetFollowCamera();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
