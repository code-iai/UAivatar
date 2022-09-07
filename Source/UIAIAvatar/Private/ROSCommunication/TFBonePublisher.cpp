// Fill out your copyright notice in the Description page of Project Settings.

#include "ROSCommunication/TFBonePublisher.h"
#include "Controller/AControllerComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

void UTFBonePublisher::SetMessageType()
{
	Topic = TEXT("/tf");
	MessageType = TEXT("tf2_msgs/TFMessage");
	MapFrameId = TEXT("map");
}

void UTFBonePublisher::SetOwner(UObject* InAgent)
{
	AActor* Agent = Cast<AActor>(InAgent);
	UAControllerComponent* ControllerComp = Agent->FindComponentByClass<UAControllerComponent>();
	Owner = Cast<UAvatarConsoleCommandController>(ControllerComp->Controller.ControllerList[ControllerName]);
	
}

void UTFBonePublisher::Publish()
{
	if (Owner)
	{
		auto CharacterMesh = Owner->Avatar->GetMesh();

		UIAIAvatarAnimationInstance* AnimationInstance = Cast<UIAIAvatarAnimationInstance>(
			CharacterMesh->GetAnimInstance());
		check(AnimationInstance != nullptr);
		check(CharacterMesh);


		FBoneContainer BoneContainer = AnimationInstance->GetRequiredBones();
		TArray<geometry_msgs::TransformStamped> StampedTransformMsgArray;
		TSharedPtr<tf2_msgs::TFMessage> TFMsgPtr =
			MakeShareable(new tf2_msgs::TFMessage());
		geometry_msgs::TransformStamped StampedTransformMsg;

		auto Mesh = CharacterMesh;
		FTransform ROSTransf;
		FString ParentFrameId;
		for (size_t i = 0; i < BoneContainer.GetNumBones(); i++)
		{
			FString linkName = Mesh->GetBoneName(i).ToString();
			if (linkName.Contains("root") ||
				linkName.Contains("pelvis") ||
				linkName.Contains("spine") ||
				linkName.Contains("head") ||
				linkName.Contains("neck") ||
				linkName.Contains("clavicle_l") ||
				linkName.Contains("clavicle_r") ||
				linkName.Contains("upperarm_l") ||
				linkName.Contains("upperarm_r") ||
				linkName.Contains("lowerarm_l") ||
				linkName.Contains("lowerarm_r") ||
				linkName.Contains("hand_l") ||
				linkName.Contains("hand_r") ||
				linkName.Contains("thigh_l") ||
				linkName.Contains("thigh_r") ||
				linkName.Contains("calf_l") ||
				linkName.Contains("calf_r") ||
				linkName.Contains("foot_l") ||
				linkName.Contains("foot_r")
			)
			{
				if (linkName.Contains("root"))
				{
					ParentFrameId = MapFrameId;

					ROSTransf = FConversions::UToROS(Mesh->GetBoneTransform(0));
				}
				else
				{
					ParentFrameId = Mesh->GetParentBone(Mesh->GetBoneName(i)).ToString();
					int32 pbone_index = Mesh->GetBoneIndex(Mesh->GetParentBone(Mesh->GetBoneName(i)));

					FTransform ChildTransform = Mesh->GetBoneTransform(i);

					FTransform ParentTransform = Mesh->GetBoneTransform(pbone_index);
					ROSTransf =
						FConversions::UToROS(
							UKismetMathLibrary::ConvertTransformToRelative(ParentTransform, ChildTransform));
				}
				FString ChildFrameId = Mesh->GetBoneName(i).ToString();

				std_msgs::Header Header;
				Header.SetSeq(Seq);
				Header.SetStamp(FROSTime());
				Header.SetFrameId(ParentFrameId);
				geometry_msgs::Transform TransfMsg(
					geometry_msgs::Vector3(ROSTransf.GetLocation()),
					geometry_msgs::Quaternion(ROSTransf.GetRotation()));


				StampedTransformMsg.SetHeader(Header);
				StampedTransformMsg.SetChildFrameId(ChildFrameId);
				StampedTransformMsg.SetTransform(TransfMsg);

				TFMsgPtr->AddTransform(StampedTransformMsg);
			}
		}

		Handler->PublishMsg(Topic, TFMsgPtr);
		Handler->Process();
		Seq++;
	}
}
