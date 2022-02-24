// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "IAIAvatarCharacter.h"
#include "Controller/AController.h"
#include "UDynamicTimeWarp.h"
#include "AvatarCRAMController.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, DefaultToInstanced, collapsecategories, hidecategories = Object, editinlinenew)
class UIAIAVATAR_API UAvatarCRAMController : public UAController
{
	GENERATED_BODY()
public:
	UAvatarCRAMController();

	virtual void Init(AActor* InModel) override;
	virtual void Tick(float InDeltaTime) override;

	virtual void NotifyQuestion();
	virtual void CheckAnswer();
	virtual void GiveAnswer(FString InAnswer);
	virtual void TimeOut();

	UPROPERTY()
	ACharacter* Avatar;
	UDTWRecognizer* HeadMoveRecognizer; 

	FTimerHandle TimeOutHandle;

	FString Question;
	FString Answer;

	FString CSVLine;
	FString FileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float timeOutLimit;

	float waitTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveCSV;

	bool bAnswering;
};
