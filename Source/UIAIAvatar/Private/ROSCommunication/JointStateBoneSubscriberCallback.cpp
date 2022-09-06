// Copyright 2022, Institute for Artificial Intelligence - University of Bremen
// Author: Mona Abdel-Keream (abdelker@uni-bremen.de)


#include "ROSCommunication/JointStateBoneSubscriberCallback.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "Conversions.h"
#include "sensor_msgs/JointState.h"
#include "IAIAvatarCharacter.h"
#include "ObjectEditorUtils.h"
#include "AnimGraphRuntime/Public/BoneControllers/AnimNode_ModifyBone.h"

FJointStateBoneSubscriberCallback::FJointStateBoneSubscriberCallback(
	const FString& InTopic, const FString& InType, UObject* InController) :
	FROSBridgeSubscriber(InTopic, InType)
{
	Controller = Cast<UAvatarConsoleCommandController>(InController);
}

/*void FJointStateBoneSubscriberCallback::UpdateBoneRotation(FRotator& BoneToRotate)
{
}*/

/*void FJointStateBoneSubscriberCallback::SetConstraintPos(USkeletalMeshComponent* targetMesh, FName constraintName, FVector newPos)
{
	if (IsValid(targetMesh)) {

		if ((targetMesh->FindConstraintInstance(constraintName)) != nullptr) {
			targetMesh->FindConstraintInstance(constraintName)->SetRefPosition(EConstraintFrame::Frame1, newPos);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("constraint not valid"));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("mesh not valid"));
	}
}*/

void FJointStateBoneSubscriberCallback::Callback(TSharedPtr<FROSBridgeMsg> Msg)
{
	TSharedPtr<sensor_msgs::JointState> JointStateMessage = StaticCastSharedPtr<sensor_msgs::JointState>(Msg);
	if (Controller)
	{
		for (int i = 0; i < JointStateMessage->Names.Num(); i++)
		{	FString ParentBoneName;
			FString ChildBoneName;

			JointStateMessage->Names[i].Split(TEXT("_to_"), &ParentBoneName, &ChildBoneName);
			auto CharacterMesh = Controller->Avatar->GetMesh();
			
			UIAIAvatarAnimationInstance * AnimationInstance = Cast<UIAIAvatarAnimationInstance>(
			CharacterMesh->GetAnimInstance());
			check(AnimationInstance != nullptr);
			check(CharacterMesh);
		
			
			FRotator Position (0, 0 ,JointStateMessage->Positions[i]);
			FRotator NewPosition = FMath::RadiansToDegrees(Position);
			FName BoneName = FName(*ChildBoneName);
			FString BoneToRotate = BoneName.ToString()+ TEXT("_Rotation");

			//FAnimNode_ModifyBone * ModifyBone = AnimationInstance->Mod;
			//ModifyBone->Rotation = NewPosition;
			//FindFProperty(AnimationInstance, FName(*BoneToRotate));
			//UpdateBoneRotation(BoneToRotate);
			//new FBodyPart_Rotations
			//MakeShared<FBodyPart_Rotations>()
			//UProperty *u_property = FBodyPart_Rotations->FindPropertyByName(FName(*BoneToRotate));
			//FBodyPart_Rotations BoneToRotate;
			const FProperty* Property =AnimationInstance->GetClass()->FindPropertyByName(FName(*BoneToRotate));

			//void* StructPtr = Property->ContainerPtrToValuePtr<void>(AnimationInstance);

			/*if(BoneToRotate == "spine_02_Rotation")
			{
				FRotator spine_02 = AnimationInstance->spine_02_Rotation;
				UE_LOG(LogTemp, Error, TEXT("Action Controller not found %s"), *spine_02.ToString());
			}*/

			
		
			FObjectEditorUtils::SetPropertyValue(AnimationInstance, FName(*BoneToRotate),NewPosition);
			//Property->SetP
			//Property.SetV
			
			
			/*
			if(BoneToRotate == "spine_02_Rotation")
			{
				FRotator spine_02 = AnimationInstance->spine_02_Rotation;
				UE_LOG(LogTemp, Error, TEXT("new position in degree: %s, new position in radians: %s"), *spine_02.ToString(), *NewPosition.ToString());
			}*/
			
			
			//FBoneContainer BoneContainer = AnimationInstance->GetRequiredBones();
			/*if ((Mesh->FindConstraintInstance(BoneName) != nullptr))
			{
				SetConstraintPos(Mesh,BoneName,Position);
			}*/
			
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Action Controller not found"));
	}
}

TSharedPtr<FROSBridgeMsg> FJointStateBoneSubscriberCallback::ParseMessage
(TSharedPtr<FJsonObject> JsonObject) const
{
	TSharedPtr<sensor_msgs::JointState> JointStateMessage =
	  MakeShareable<sensor_msgs::JointState>(new sensor_msgs::JointState());

	JointStateMessage->FromJson(JsonObject);

	return StaticCastSharedPtr<FROSBridgeMsg>(JointStateMessage);
}

FJointStateBoneSubscriberCallback::~FJointStateBoneSubscriberCallback()
{
}
