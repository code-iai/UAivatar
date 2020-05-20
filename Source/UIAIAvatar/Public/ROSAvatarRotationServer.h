// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeSrvServer.h"
#include "Misc/Base64.h"
#include "iai_avatar_msgs/AvatarRotation.h"
#include "IAIAvatarCharacter.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "UIAIAvatarModule.h"

class FROSAvatarRotationServer final : public FROSBridgeSrvServer
{
public:
	FROSAvatarRotationServer(AActor* InOwner, const FString& InName, const FString& InType):
        FROSBridgeSrvServer(InName, InType)
    {
		Owner = InOwner;
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
    {
        TSharedPtr<iai_avatar_msgs::AvatarRotation::Request> Request =
            MakeShareable(new iai_avatar_msgs::AvatarRotation::Request());
        Request->FromJson(JsonObject);
        return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
    }

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
    {

		UE_LOG(LogAvatarROS, Log, TEXT("Service Request!"));
        TSharedPtr<iai_avatar_msgs::AvatarRotation::Request> Request =
            StaticCastSharedPtr<iai_avatar_msgs::AvatarRotation::Request>(InRequest);

  
        UE_LOG(LogAvatarROS, Log, TEXT("[%s] In actor %s: Service [%s] Server: Rotate to RPY %f %f %f"),
			*FString(__FUNCTION__), *Owner->GetName(), *Name, 
			Request->GetRotation().GetX(),
			Request->GetRotation().GetY(), 
			Request->GetRotation().GetZ());
		
		//// Execute on game thread // roll is last in the constructor
		FRotator NewRot(Request->GetRotation().GetY(), Request->GetRotation().GetZ(),   Request->GetRotation().GetX());

		AsyncTask(ENamedThreads::GameThread, [this, NewRot]()
		{
			// Search for a static mesh actor with the name cube and move it up
			UE_LOG(LogAvatarROS, Log, TEXT("[%s] Executing on game thread."), *FString(__FUNCTION__));
			UE_LOG(LogAvatarROS, Error, TEXT("Rotation is %f %f %f"),
				NewRot.Roll, NewRot.Pitch, NewRot.Yaw
			);

			check(Owner);

			AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);

			UE_LOG(LogAvatarROS, Log, TEXT("About to execute  on %s"), *(Avatar->GetName()) );

			Avatar->SetAbsoluteActorRotationWithTimeline(NewRot);
		}
		);
			  UE_LOG(LogAvatarROS, Log, TEXT("Returning response in FROSAvatarRotationServer"));

        return MakeShareable<FROSBridgeSrv::SrvResponse>
                (new iai_avatar_msgs::AvatarRotation::Response());
    }

private:
	AActor* Owner;
};
