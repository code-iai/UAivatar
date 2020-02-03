// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "AvatarConsoleCommandController.h"

void UAvatarConsoleCommandController::Init(AActor* InAgent)
{
	if (!InAgent)
	{
		UE_LOG(LogTemp, Error, TEXT("Avatar Console Command Controller not attached to an Avatar"));
	}

	Avatar = Cast<AIAIAvatarCharacter>(InAgent);
	check(Avatar != nullptr);

	bSuccess = false;
}

void UAvatarConsoleCommandController::Tick(float InDeltaTime) {}

void UAvatarConsoleCommandController::SendConsoleCommand() {

	Avatar->ProcessConsoleCommand(Command);

	Message = "Command Sent";
	bSuccess = true;
}