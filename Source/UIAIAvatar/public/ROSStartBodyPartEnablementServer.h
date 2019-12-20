// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeSrvServer.h"
#include "Misc/Base64.h"
#include "iai_avatar_msgs/NamedTargetInterpolation.h"
#include "IAIAvatarCharacter.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "UIAIAvatarModule.h"

class FROSStartBodyPartEnablementServer final : public FROSBridgeSrvServer
{
public:
	FROSStartBodyPartEnablementServer(AActor* InOwner, const FString& InName, const FString& InType):
        FROSBridgeSrvServer(InName, InType)
    {
		Owner = InOwner;
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
    {
        TSharedPtr<iai_avatar_msgs::NamedTargetInterpolation::Request> Request =
            MakeShareable(new iai_avatar_msgs::NamedTargetInterpolation::Request());
        Request->FromJson(JsonObject);
        return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
    }

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
    {

		UE_LOG(LogAvatarROS, Log, TEXT("Service Request!"));
        TSharedPtr<iai_avatar_msgs::NamedTargetInterpolation::Request> Request =
            StaticCastSharedPtr<iai_avatar_msgs::NamedTargetInterpolation::Request>(InRequest);

  
        UE_LOG(LogAvatarROS, Log, TEXT("[%s] In actor %s: Service [%s] Server: Vector %f %f %f"),
			*FString(__FUNCTION__), *Owner->GetName(), *Name, 
			Request->GetTarget().GetX(),
			Request->GetTarget().GetY(),
			Request->GetTarget().GetZ() );
		
		FVector NewVec(Request->GetTarget().GetX(), Request->GetTarget().GetY(),   Request->GetTarget().GetZ());
		FString M_Name(Request->GetName());

		AsyncTask(ENamedThreads::GameThread, [this, NewVec, M_Name]()
		{
			// Search for a static mesh actor with the name cube and move it up
			UE_LOG(LogAvatarROS, Log, TEXT("[%s] Executing on game thread."), *FString(__FUNCTION__));
			UE_LOG(LogAvatarROS, Error, TEXT("New Vec is %f %f %f"),
				NewVec.X, NewVec.Y, NewVec.Z
			);

			check(Owner);

			AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);

			UE_LOG(LogAvatarROS, Log, TEXT("About to execute  on %s"), *(Avatar->GetName()) );

			if (M_Name.Equals("spine")) {
				Avatar->StartSpineEnablement(FRotator(NewVec.Y, NewVec.Z, NewVec.X));
			}
			else if (M_Name.Equals("left_hand_rotation"))
			{
				Avatar->StartLeftHandRotationEnablement(FRotator(NewVec.Y, NewVec.Z, NewVec.X));
			}
			else if (M_Name.Equals("left_hand_ik"))
			{
				Avatar->StartLeftHandIKEnablement(NewVec);
			}
			else if (M_Name.Equals("right_hand_rotation"))
			{
				Avatar->StartRightHandRotationEnablement(FRotator(NewVec.Y, NewVec.Z, NewVec.X));
			}
			else if (M_Name.Equals("right_hand_ik"))
			{
				Avatar->StartRightHandIKEnablement(NewVec);
			}
			else
			{
				UE_LOG(LogAvatarROS, Warning, TEXT("Called StartBodyPartEnablement without known name %s"), *M_Name);
			}
			// Avatar->InterpolateLeftHandIKTo(NewVec);
			//Avatar->SetAbsoluteActorRotationWithTimeline(NewVec);
		}
		);

        return MakeShareable<FROSBridgeSrv::SrvResponse>
                (new iai_avatar_msgs::NamedTargetInterpolation::Response());
    }

private:
	AActor* Owner;
};
