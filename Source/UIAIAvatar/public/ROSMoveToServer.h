// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeSrvServer.h"
#include "Misc/Base64.h"
#include "iai_avatar_msgs/MoveTo.h"
#include "IAIAvatarCharacter.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "UIAIAvatarModule.h"

class FROSMoveToServer final : public FROSBridgeSrvServer
{
public:
	FROSMoveToServer(AActor* InOwner, const FString& InName, const FString& InType):
        FROSBridgeSrvServer(InName, InType)
    {
		Owner = InOwner;
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
    {
        TSharedPtr<iai_avatar_msgs::MoveTo::Request> Request =
            MakeShareable(new iai_avatar_msgs::MoveTo::Request());
        Request->FromJson(JsonObject);
        return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
    }

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
    {

		UE_LOG(LogAvatarROS, Log, TEXT("MoveTo Service Request!"));
        TSharedPtr<iai_avatar_msgs::MoveTo::Request> Request =
            StaticCastSharedPtr<iai_avatar_msgs::MoveTo::Request>(InRequest);

  
        UE_LOG(LogAvatarROS, Log, TEXT("[%s] In actor %s: Service [%s] Server: Move to Coords %f %f %f"),
			*FString(__FUNCTION__), *Owner->GetName(), *Name, 
			Request->GetPosition().GetX(),
			Request->GetPosition().GetY(), 
			Request->GetPosition().GetZ());
		//
		//// Execute on game thread
		FVector NewPos(Request->GetPosition().GetX(), Request->GetPosition().GetY(), Request->GetPosition().GetZ());

		//UE_LOG(LogTemp, Error, TEXT("Vector is %f %f %f"),
		//	NewPos.X, NewPos.Y, NewPos.Z
		//	);

		AsyncTask(ENamedThreads::GameThread, [this, NewPos]()
		{
			// Search for a static mesh actor with the name cube and move it up
			UE_LOG(LogAvatarROS, Log, TEXT("[%s] Executing on game thread."), *FString(__FUNCTION__));
			UE_LOG(LogAvatarROS, Error, TEXT("Vector is %f %f %f"),
				NewPos.X, NewPos.Y, NewPos.Z
			);

			check(Owner);

			AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);

			UE_LOG(LogAvatarROS, Log, TEXT("About to execute MoveTo on %s"), *(Avatar->GetName()) );

			Avatar->MoveTo(NewPos);
		}
		);
			  UE_LOG(LogAvatarROS, Log, TEXT("Returning response in FROSAvatarRotationServer"));

        return MakeShareable<FROSBridgeSrv::SrvResponse>
                (new iai_avatar_msgs::MoveTo::Response());
    }

private:
	AActor* Owner;
};
