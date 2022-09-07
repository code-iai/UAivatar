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

void FJointStateBoneSubscriberCallback::Callback(TSharedPtr<FROSBridgeMsg> Msg)
{
	TSharedPtr<sensor_msgs::JointState> JointStateMessage = StaticCastSharedPtr<sensor_msgs::JointState>(Msg);
	if (Controller)
	{
		for (int i = 0; i < JointStateMessage->Names.Num(); i++)
		{
			FString ParentBoneName;
			FString ChildBoneName;

			JointStateMessage->Names[i].Split(TEXT("_to_"), &ParentBoneName, &ChildBoneName);
			auto CharacterMesh = Controller->Avatar->GetMesh();

			UIAIAvatarAnimationInstance* AnimationInstance = Cast<UIAIAvatarAnimationInstance>(
				CharacterMesh->GetAnimInstance());
			check(AnimationInstance != nullptr);
			check(CharacterMesh);

			FRotator Position(0, 0, JointStateMessage->Positions[i]);
			FRotator NewPosition = FMath::RadiansToDegrees(Position);
			FName BoneName = FName(*ChildBoneName);
			FString BoneToRotate = BoneName.ToString() + TEXT("_Rotation");

			const FProperty* Property = AnimationInstance->GetClass()->FindPropertyByName(FName(*BoneToRotate));

			if (Property)
			{
				FObjectEditorUtils::SetPropertyValue(AnimationInstance, FName(*BoneToRotate), NewPosition);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Property %s does not exist"), *Property->GetName());
			}
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
