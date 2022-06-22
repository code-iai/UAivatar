// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "Controller/AvatarPerceiveObjController.h"
#include "Engine.h"

void UAvatarPerceiveObjController::Init(AActor* InAgent)
{
	if (!InAgent)
	{
		UE_LOG(LogTemp, Error, TEXT("Avatar Perceive Object Controller not attached to an Avatar"));
	}

	Avatar = Cast<AIAIAvatarCharacter>(InAgent);
	check(Avatar != nullptr);	
}

UAvatarPerceiveObjController::UAvatarPerceiveObjController() {

}

void UAvatarPerceiveObjController::Tick(float InDeltaTime) {}

void UAvatarPerceiveObjController::PerceiveObject()
{
	if (bActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerceiveObject is a dummy function"));
		GoalStatusList.Last().Status = 1;

		bool bObjectFound = false;
		TArray<AActor*> found_actors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*TypeToPerceive.ToLower()), found_actors);

		for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ActorItr->ActorHasTag(FName( *TypeToPerceive.ToLower() )))
			{
				bObjectFound = true;
				ObjectName = ActorItr->GetName();
				ObjectPoseWorld.SetLocation(ActorItr->GetTransform().GetLocation());
				ObjectPoseWorld.SetRotation(ActorItr->GetTransform().GetRotation());
				break;
			}
		}

		if (bObjectFound)
		{

			check(Avatar->GetMesh());

			// Get Animation
			UIAIAvatarAnimationInstance *AnimationInstance = Cast<UIAIAvatarAnimationInstance>(Avatar->GetMesh()->GetAnimInstance());
			check(AnimationInstance != nullptr);

			FTransform ReferenceTransform = Avatar->GetMesh()->GetComponentTransform();

			FVector LocalLocation = ObjectPoseWorld.GetLocation() - ReferenceTransform.GetLocation();
			FVector LocalPose = ReferenceTransform.GetRotation().Inverse().RotateVector(LocalLocation);
			FQuat LocalRotator = ObjectPoseWorld.GetRotation() * ReferenceTransform.GetRotation().Inverse();

			ObjectPose.SetLocation(LocalPose);
			ObjectPose.SetRotation(LocalRotator);

			GoalStatusList.Last().Status = 3;
			bActive = false;
			bPublishResult = true;
			// UE_LOG(LogTemp, Warning, TEXT("Base: %s, :World %s"), *Pose.ToString(), *TempRotator.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No Object of Type %s found"), *TypeToPerceive);
		}
	}
}

