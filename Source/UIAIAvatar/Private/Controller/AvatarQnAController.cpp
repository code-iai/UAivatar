// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "Controller/AvatarQnAController.h"
#include "Engine.h"
/*
void UAvatarQnAController::Init(AActor* InAgent)
{
	if (!InAgent)
	{
		UE_LOG(LogTemp, Error, TEXT("Avatar QnA Controller not attached to an Avatar"));
	}

	Avatar = Cast<ACharacter>(InAgent);
	check(Avatar != nullptr);	

	HeadMoveRecognizer = Avatar->FindComponentByClass<UDTWRecognizer>();

}

UAvatarQnAController::UAvatarQnAController()
{
	bAnswering = false;
	CSVLine= "";
	FileName = "Preferences." + FDateTime::Now().ToString() + ".csv";
	timeOutLimit = 6;
	waitTime = 0;
}

void UAvatarQnAController::Tick(float InDeltaTime)
{
	CancelAction();
	if (bActive)
	{
		waitTime += InDeltaTime;
		GoalStatusList.Last().Status = 1;

		if (HeadMoveRecognizer->bRecording) {
			if (!bAnswering) {
				waitTime = 0;
				bAnswering = true;
			}
		}

		if (bAnswering) {
			if (!HeadMoveRecognizer->bRecording) {
				CheckAnswer();
			}
		}
		if (waitTime > timeOutLimit) {
			TimeOut();
		}
	}
	if (bCancel) {
		bAnswering = false;
		waitTime = 0;
	}
}

void UAvatarQnAController::NotifyQuestion()
{
	
	FTimerDelegate TimeOutDelegate;

	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::Green, FString::Printf(TEXT("* %s"), *Question), true, FVector2D(4, 4));

	bActive = true;
	if (bSaveCSV) {
		CSVLine = Avatar->GetName() + ",";
		CSVLine += Question + ",";
	}
}

void UAvatarQnAController::CheckAnswer()
{
	FString AnswerText;
	int8 decision;
	decision = HeadMoveRecognizer->GetDecision();

	switch (decision)
	{
	case 0:
		AnswerText = "Yes";
		break;
	case 1:
		AnswerText = "No";
		break;
	case 2:
		AnswerText = "Thinking";
		break;
	default:
		AnswerText = "Unknown";
	}

	GiveAnswer(AnswerText);
}

void UAvatarQnAController::GiveAnswer(FString InAnswer)
{
	GoalStatusList.Last().Status = 3;
	waitTime = 0;
	bActive = false;
	bPublishResult = true;
	bAnswering = false;
	Answer = InAnswer;

	if (bSaveCSV) {
		CSVLine += Answer + "\n";

		FString FileString;
		FFileHelper::LoadFileToString(FileString, *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
		FileString += CSVLine;
		FFileHelper::SaveStringToFile(FileString, *(FPaths::ProjectDir() + FString("Datatables/") + FileName));
	}
}

void UAvatarQnAController::TimeOut()
{
	GiveAnswer(TEXT("Time Out"));
} */