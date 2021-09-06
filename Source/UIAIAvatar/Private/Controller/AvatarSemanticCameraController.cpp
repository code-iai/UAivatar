// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "Controller/AvatarSemanticCameraController.h"

void UAvatarSemanticCameraController::Init(AActor* InAgent)
{
	if (!InAgent)
	{
		UE_LOG(LogTemp, Error, TEXT("Avatar Semantic Camera Controller not attached to an Avatar"));
	}

	Avatar = Cast<AIAIAvatarCharacter>(InAgent);
	check(Avatar != nullptr);

	bSuccess = false;
}

void UAvatarSemanticCameraController::Tick(float InDeltaTime) {}

void UAvatarSemanticCameraController::ListObjects() {

	TMap<FString, FHitResult> MyUniqueHits;

	MyUniqueHits = Avatar->ListObjects();

	Objects.Empty();
	for (auto& Item : MyUniqueHits)
	{
		FSemanticObject Object;
		Object.Name = Item.Key;
		for (auto& Tag :Item.Value.GetActor()->Tags )
			Object.Tags.Add(Tag.ToString());
		Objects.Add(Object);
	}

	Message = FString::Printf(TEXT("%d Items found"), MyUniqueHits.Num());
	bSuccess = true;
}