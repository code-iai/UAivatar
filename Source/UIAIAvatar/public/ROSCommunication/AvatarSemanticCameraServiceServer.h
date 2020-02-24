#pragma once

#include "ROSBridgeSrvServer.h"
#include "iai_avatar_msgs/SemanticCamera.h"
#include "Controller/AvatarSemanticCameraController.h"

class FAvatarSemanticCameraServer final : public FROSBridgeSrvServer
{
public:

	FAvatarSemanticCameraServer(const FString& InName, const FString& InType, UAController* InOwner) :
				FROSBridgeSrvServer(InName, InType)
	{
		Owner = Cast<UAvatarSemanticCameraController>(InOwner);
	}

	UAvatarSemanticCameraController* Owner;

	TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override;
	TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override;
};