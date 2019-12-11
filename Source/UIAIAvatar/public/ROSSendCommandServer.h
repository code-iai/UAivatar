// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeSrvServer.h"
#include "Misc/Base64.h"
#include "iai_avatar_msgs/Command.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "IAIAvatarROSIntegrationComponent.h"

#include "UIAIAvatarModule.h"

class FROSSendCommandServer final : public FROSBridgeSrvServer
{
public:
	FROSSendCommandServer(AActor* InOwner, const FString& InName, const FString& InType):
        FROSBridgeSrvServer(InName, InType)
    {
		Owner = InOwner;
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
    {
        TSharedPtr<iai_avatar_msgs::Command::Request> Request =
            MakeShareable(new iai_avatar_msgs::Command::Request());
        Request->FromJson(JsonObject);
        return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
    }

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
    {

		UE_LOG(LogAvatarROS, Log, TEXT("Service Request!"));
        TSharedPtr<iai_avatar_msgs::Command::Request> Request =
            StaticCastSharedPtr<iai_avatar_msgs::Command::Request>(InRequest);

        UE_LOG(LogAvatarROS, Log, TEXT("[%s] In actor %s: Service [%s] Server: Received the following command: %s"),
			*FString(__FUNCTION__), *Owner->GetName(), *Name, 
			*(Request->GetCommand())
		);


		FString RequestString = Request->GetCommand();
		//FRotator NewRotation(0, Request->GetPan(), Request->GetTilt());

		//
		//// Execute on game thread
		//FVector NewPos(Request->GetPosition().GetX(), Request->GetPosition().GetY(), Request->GetPosition().GetZ());

		//UE_LOG(LogTemp, Error, TEXT("Vector is %f %f %f"),
		//	NewPos.X, NewPos.Y, NewPos.Z
		//	);

		 AsyncTask(ENamedThreads::GameThread, [this, RequestString]()
		 {
			 UE_LOG(LogTemp, Log, TEXT("[%s] Executing on game thread."), *FString(__FUNCTION__));	

			 if (UIAIAvatarROSIntegrationComponent* ResponsibleROSIntegrationComponent
				 = Owner->FindComponentByClass<UIAIAvatarROSIntegrationComponent>()) {
				 // It seems that this calls all interested parties in the current thread
				 ResponsibleROSIntegrationComponent->TriggerROSSendCommandRequest(RequestString);
			 }
			 else {
				 UE_LOG(LogAvatarROS, Error,
					 TEXT("Can't forward Send Command Request to ROSIntegrationComponent. Couldn't find a UIAIAvatarROSIntegrationComponent on %s. Maybe you've not added a UIAIAvatarROSIntegrationComponent to the Actor."), *Owner->GetName()
				 );

			 }
		 }
		 );

        return MakeShareable<FROSBridgeSrv::SrvResponse>
                (new iai_avatar_msgs::Command::Response());
    }

private:
	AActor* Owner;
};
