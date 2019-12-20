// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "ROSBridgeSrvServer.h"
#include "Misc/Base64.h"
#include "iai_avatar_msgs/GeneralGrasp.h"
#include "IAIAvatarCharacter.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "UIAIAvatarModule.h"

class FROSGeneralGraspServer final : public FROSBridgeSrvServer
{
public:

	TArray<FString> OutList;
	bool outSuccess = false;

	FROSGeneralGraspServer(AActor* InOwner, const FString& InName, const FString& InType):
        FROSBridgeSrvServer(InName, InType)
    {
		Owner = InOwner;
    }

    TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
    {
        TSharedPtr<iai_avatar_msgs::GeneralGrasp::Request> Request =
            MakeShareable(new iai_avatar_msgs::GeneralGrasp::Request());
        Request->FromJson(JsonObject);
        return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
    }

    TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
    {

		// Local Variables
		TSharedPtr<iai_avatar_msgs::GeneralGrasp::Request> Request =
			StaticCastSharedPtr<iai_avatar_msgs::GeneralGrasp::Request>(InRequest);
		
		FString command(Request->GetCommand());
		FString targetObject(Request->GetTargetobject());

		UE_LOG(LogAvatarROS, Log, TEXT("Service Request!"));
		UE_LOG(LogAvatarROS, Log, TEXT("[%s] In actor %s: Service [%s] Command: %s -> \"%s\""),
			*FString(__FUNCTION__), *Owner->GetName(), *Name, *command, *targetObject);

		AsyncTask(ENamedThreads::GameThread, [this, targetObject, command]()
		{
			check(Owner);
			AIAIAvatarCharacter* Avatar = Cast<AIAIAvatarCharacter>(Owner);
			TArray<FString> objectList;

			UE_LOG(LogAvatarROS, Log, TEXT("[%s] Executing on game thread."), *FString(__FUNCTION__));
					   
			if (command.Equals("list_n_grasp")) {
				this->OutList = Avatar->GraspTargetObject_ROS(targetObject);
				for (auto It = objectList.CreateIterator(); It; ++It) {
					UE_LOG(LogAvatarROS, Log, TEXT("ROS OBJECT LIST %s"), *(*It));
				}

				if ((this->OutList).Find(targetObject) != INDEX_NONE) {
					this->outSuccess = true;
				}
				else {
					this->outSuccess = false;
				}
			}

			else if (command.Equals("stop_ggrasp"))
			{
				Avatar->StopHandReach();
			}
			else if (command.Equals("detach_left_hand"))
			{
				Avatar->DetachGraspedObject_l();
			}
			else if (command.Equals("detach_right_hand"))
			{
				Avatar->DetachGraspedObject_r();
			}
			else
			{
				UE_LOG(LogAvatarROS, Warning, TEXT("Called GeneralGrasp without known command %s"), *Name);
			}
			
		}
		);

		return MakeShareable<FROSBridgeSrv::SrvResponse> (
			new iai_avatar_msgs::GeneralGrasp::Response(this->outSuccess, this->OutList));
    }

private:
	AActor* Owner;
};
