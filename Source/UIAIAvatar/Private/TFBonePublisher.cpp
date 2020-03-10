// Fill out your copyright notice in the Description page of Project Settings.

#include "TFBonePublisher.h"
#include "ROSBridgeGameInstance.h"
#include "UIAIAvatarModule.h" // include for log levels
#include "Math/TransformNonVectorized.h"
#include "Classes/Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "IAIAvatarCharacter.h"
#include "Components/SkinnedMeshComponent.h"
#include "IAIAvatarAnimationInstance.h"
#include "Conversions.h"

// Sets default values for this component's properties
UTFBonePublisher::UTFBonePublisher()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

	CurrentDeltaForPublishRate = 0;
}


// Called when the game starts
void UTFBonePublisher::BeginPlay()
{
    Super::BeginPlay();


    // Create the ROSBridge handler for connecting with ROS
    //ROSBridgeHandler = MakeShareable<FROSBridgeHandler>(
           // new FROSBridgeHandler(ServerIP, ServerPORT));

    // Create the tf publisher
    TFPublisher = MakeShareable<FROSBridgePublisher>(
            new FROSBridgePublisher("tf", "tf2_msgs/TFMessage"));

    // Getting the actual UROSBridgeGameInstance
    UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetOwner()->GetGameInstance());
    check(ActiveGameInstance);

    if (!ActiveGameInstance->ROSHandler.IsValid() || !ActiveGameInstance->ROSHandler->IsClientConnected())
    {
        UE_LOG(LogAvatarROS, Error, TEXT("Not connected to ROS. Can't initialize IAIAvatarROSIntegrationComponent on %s"), *(GetOwner()->GetName()) );
        return;
    }

    ActiveGameInstance->ROSHandler->AddPublisher(TFPublisher);
    //TFPublisher = MakeShareable<FROSBridgePublisher>(
    //	new FROSBridgePublisher("tf_static", "tf2_msgs/TFMessage"));

    // Connect to ROS
    //ROSBridgeHandler->Connect();

    // Add publisher
    //ROSBridgeHandler->AddPublisher(TFPublisher);


}


// Called every frame
void UTFBonePublisher::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentDeltaForPublishRate += DeltaTime;
	if (CurrentDeltaForPublishRate < ConstantPublishRate) 
		return;

	// We've exceeded deltatime. Reset counter and execute tf
	CurrentDeltaForPublishRate = 0;

    FROSTime TimeNow = FROSTime::Now();

    UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetOwner()->GetGameInstance());
    check(ActiveGameInstance);

    if (!ActiveGameInstance->ROSHandler.IsValid() || !ActiveGameInstance->ROSHandler->IsClientConnected())
    {
        UE_LOG(LogAvatarROS, Error, TEXT("Not connected to ROS. Can't initialize IAIAvatarROSIntegrationComponent on %s"), *(GetOwner()->GetName()) );
        return;
    }

    ACharacter* myCharacter = Cast<ACharacter>(GetOwner());
    USkeletalMeshComponent* CharacterMesh = myCharacter->GetMesh();

    UIAIAvatarAnimationInstance * AnimationInstance = Cast<UIAIAvatarAnimationInstance>(
            CharacterMesh->GetAnimInstance());
    check(AnimationInstance != nullptr);
    check(CharacterMesh);

    auto Mesh = CharacterMesh;


    FBoneContainer BoneContainer = AnimationInstance->GetRequiredBones();
    //UE_LOG(LogTemp, Error, TEXT("Count of Bones in AnimationInstance %d"), BoneContainer.GetNumBones());

    TArray <geometry_msgs::TransformStamped> StampedTransformMsgArray;



    for (size_t i = 0; i < BoneContainer.GetNumBones(); i++) {

        if (i == 0) {
            StampedTransformMsgArray.Add(GetTransformStampedMsgRoot(TimeNow, Seq))  ;
        }

        else{

            StampedTransformMsgArray.Add(GetTransformStampedMsg(TimeNow, Seq, i))  ;
        }

        if ( i == BoneContainer.GetNumBones()-1){

            TSharedPtr<tf2_msgs::TFMessage> TFMsgPtr =
                    MakeShareable(new tf2_msgs::TFMessage());

            TFMsgPtr->AppendTransforms(StampedTransformMsgArray);


            // Publish
            //ROSBridgeHandler->PublishMsg("/tf", TFMsgPtr);

            ActiveGameInstance->ROSHandler->PublishMsg("/tf", TFMsgPtr);
            ActiveGameInstance->ROSHandler->Process();
        }



    }
    // Update message sequence
    Seq++;
}
////
geometry_msgs::TransformStamped UTFBonePublisher::GetTransformStampedMsg(const FROSTime& InTime, const uint32 InSeq, const uint32 bone_index) const {
    geometry_msgs::TransformStamped StampedTransformMsg;

    //const FString ParentFrameId = TFRootFrameName ;
    //const FString ParentFrameId = "avatar";
    //Header.SetFrameId(ParentFrameId);

    ACharacter* myCharacter = Cast<ACharacter>(GetOwner());

    //USkeletalMeshComponent* CharacterMesh = GetMesh();
    USkeletalMeshComponent* CharacterMesh = myCharacter->GetMesh();

    UIAIAvatarAnimationInstance * AnimationInstance = Cast<UIAIAvatarAnimationInstance>(
            CharacterMesh->GetAnimInstance());
    check(AnimationInstance != nullptr);
    check(CharacterMesh);

    auto Mesh = CharacterMesh;

    std_msgs::Header Header;
    Header.SetSeq(InSeq);
    Header.SetStamp(InTime);



    FName Bone = Mesh->GetBoneName(bone_index);
    FString FrameId = Bone.ToString();

    FName ParentBone = Mesh->GetParentBone(Bone);
    int32 pbone_index = Mesh->GetBoneIndex(ParentBone);
    FString ParentFrameId = ParentBone.ToString();

    FTransform ChildTransform = Mesh->GetBoneTransform(bone_index);
    FTransform ParentTransform = Mesh->GetBoneTransform(pbone_index);



    //// Transform to ROS coordinate system

    //FTransform ROSTransf = FConversions::UToROS(
    //(myCharacter->GetTransform() * Mesh->GetBoneTransform(bone_index).Inverse()).Inverse());
    //FTransform ROSTransf = FConversions::UToROS(Mesh->GetBoneTransform(bone_index));

    //FTransform ROSTransf = FConversions::UToROS(bone_transform.SetToRelativeTransform(Mesh->GetBoneTransform(pbone_index)));

    //FTransform ROSTransf = FConversions::UToROS(UKismetMathLibrary::ConvertTransformToRelative(ParentTransform,ChildTransform));
	FTransform ROSTransf = FConversions::UToROS(UKismetMathLibrary::MakeRelativeTransform(ParentTransform, ChildTransform));

    geometry_msgs::Transform TransfMsg(
            geometry_msgs::Vector3(ROSTransf.GetLocation()),
            geometry_msgs::Quaternion(ROSTransf.GetRotation()));

    Header.SetFrameId(ParentFrameId);

    StampedTransformMsg.SetHeader(Header);
    StampedTransformMsg.SetChildFrameId(FrameId);
    StampedTransformMsg.SetTransform(TransfMsg);


    return StampedTransformMsg;




}

geometry_msgs::TransformStamped UTFBonePublisher::GetTransformStampedMsgRoot(const FROSTime& InTime, const uint32 InSeq) const {
    geometry_msgs::TransformStamped StampedTransformMsg;


    const FString ParentFrameId = TFRootFrameName ;
    //const FString ParentFrameId = "avatar";

    ACharacter* myCharacter = Cast<ACharacter>(GetOwner());

    //USkeletalMeshComponent* CharacterMesh = GetMesh();
    USkeletalMeshComponent* CharacterMesh = myCharacter->GetMesh();

    UIAIAvatarAnimationInstance * AnimationInstance = Cast<UIAIAvatarAnimationInstance>(
            CharacterMesh->GetAnimInstance());
    check(AnimationInstance != nullptr);
    check(CharacterMesh);

    auto Mesh = CharacterMesh;

    std_msgs::Header Header;
    Header.SetSeq(InSeq);
    Header.SetStamp(InTime);
    FName Bone = Mesh->GetBoneName(0);
    FString FrameId = Bone.ToString();

    //FTransform ChildTransform = Mesh->GetBoneTransform(bone_index);
    //FTransform ParentTransform = Mesh->GetBoneTransform(pbone_index);

    //// Transform to ROS coordinate system

    // FTransform ROSTransf = FConversions::UToROS(
    //(myCharacter->GetTransform() * Mesh->GetBoneTransform(0).Inverse()).Inverse());
    FTransform ROSTransf = FConversions::UToROS(Mesh->GetBoneTransform(0));



    geometry_msgs::Transform TransfMsg(
            geometry_msgs::Vector3(ROSTransf.GetLocation()),
            geometry_msgs::Quaternion(ROSTransf.GetRotation()));

    Header.SetFrameId(ParentFrameId);

    StampedTransformMsg.SetHeader(Header);
    StampedTransformMsg.SetChildFrameId(FrameId);
    StampedTransformMsg.SetTransform(TransfMsg);


    return StampedTransformMsg;


}






