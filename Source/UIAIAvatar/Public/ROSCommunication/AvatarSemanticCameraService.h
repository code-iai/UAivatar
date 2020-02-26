// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSCommunication/ROSService.h"
#include "Controller/AControllerComponent.h"
#include "ROSCommunication/AvatarSemanticCameraServiceServer.h"
#include "AvatarSemanticCameraService.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API UAvatarSemanticCameraService : public UROSService
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
