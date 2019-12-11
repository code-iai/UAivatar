// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeSrvServer.h"
#include "Misc/Base64.h"
#include "iai_avatar_msgs/PanTilt.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"

class FROSMoveHeadServer final : public FROSBridgeSrvServer
{
public:
	FROSMoveHeadServer(AActor* InOwner, const FString& InName, const FString& InType):
        FROSBridgeSrvServer(InName, InType)
    {
		Owner = InOwner;
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
    {
        TSharedPtr<iai_avatar_msgs::PanTilt::Request> Request =
            MakeShareable(new iai_avatar_msgs::PanTilt::Request());
        Request->FromJson(JsonObject);
        return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
    }

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
    {

		UE_LOG(LogTemp, Log, TEXT("Service Request!"));
        TSharedPtr<iai_avatar_msgs::PanTilt::Request> Request =
            StaticCastSharedPtr<iai_avatar_msgs::PanTilt::Request>(InRequest);

        UE_LOG(LogTemp, Log, TEXT("[%s] In actor %s: Service [%s] Server: Add Two Ints: %f %f"),
			*FString(__FUNCTION__), *Owner->GetName(), *Name, 
			Request->GetPan(),
			Request->GetTilt());

		FRotator NewRotation(0, Request->GetPan(), Request->GetTilt());

		//
		//// Execute on game thread
		//FVector NewPos(Request->GetPosition().GetX(), Request->GetPosition().GetY(), Request->GetPosition().GetZ());

		//UE_LOG(LogTemp, Error, TEXT("Vector is %f %f %f"),
		//	NewPos.X, NewPos.Y, NewPos.Z
		//	);

		 AsyncTask(ENamedThreads::GameThread, [this, NewRotation]()
		 {
			 UE_LOG(LogTemp, Log, TEXT("[%s] Executing on game thread."), *FString(__FUNCTION__));			
			 UE_LOG(LogTemp, Error, TEXT("FRotator is %f %f %f"),
				 NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll
			 );

			 check(Owner);

			 AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);

			 UE_LOG(LogTemp, Log, TEXT("About to execute MoveHead on %s"), *(Avatar->GetName()) );

			 Avatar->MoveHead(NewRotation);
		 }
		 );

        return MakeShareable<FROSBridgeSrv::SrvResponse>
                (new iai_avatar_msgs::PanTilt::Response());
    }

private:
	AActor* Owner;
};
