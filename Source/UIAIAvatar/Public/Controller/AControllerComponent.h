// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AController.h"
#include "AControllerComponent.generated.h"

USTRUCT(Blueprintable)
struct FAControllerContainer
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, export, noclear)
		TMap<FString, UAController*> ControllerList;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIAIAVATAR_API UAControllerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAControllerComponent();
	~UAControllerComponent();

	virtual void ExcecuteCommands(TArray<FString> InCommands);
	virtual void ExcecuteCommands();
	virtual void ExcecuteCommand(FString InCommand);

	virtual UAController* ControllerList(FString ControllerName);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	FAControllerContainer Controller;

	TQueue<FString, EQueueMode::Mpsc> CommandQuerry;
protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	AActor* Agent;
};
