// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "Controller/AvatarConsoleCommandController.h"

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

	//Avatar->ProcessConsoleCommand(Command);

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Executing on game thread."), *FString(__FUNCTION__));
		this->Avatar->ProcessConsoleCommand(this->Command);
		UE_LOG(LogTemp, Log, TEXT("[%s] Game thread finished."), *FString(__FUNCTION__));
	}
	);

	Message = "Command Sent";
	bSuccess = true;
}