// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSService.h"
#include "Controller/AControllerComponent.h"
#include "Controller/AvatarConsoleCommandController.h"
#include "ROSCommunication/AvatarConsoleCommandServiceServer.h"
#include "AvatarConsoleCommandService.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAvatarConsoleCommandService : public UROSService
{
	GENERATED_BODY()

public:


protected:
	UPROPERTY()
		UAControllerComponent* Owner;

	virtual void SetOwner(UObject* InOwner);
	virtual void SetType();
	virtual void CreateServiceServer();
};
