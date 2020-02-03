#pragma once

#include "ROSBridgeSrvServer.h"
#include "iai_avatar_msgs/Command.h"
#include "Controller/AvatarConsoleCommandController.h"

class FAvatarConsoleCommandsServer final : public FROSBridgeSrvServer
{
public:

	// TSharedPtr<FROSBridgeHandler> Handler;
	FAvatarConsoleCommandsServer(const FString& InName, const FString& InType, UObject* InOwner) :
				FROSBridgeSrvServer(InName, InType)
	{
		Owner = Cast<UAvatarConsoleCommandController>(InOwner);
	}

	UAvatarConsoleCommandController* Owner;

	TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override;
	TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override;
};