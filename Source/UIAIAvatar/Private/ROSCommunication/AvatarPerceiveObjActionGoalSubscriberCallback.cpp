#include "ROSCommunication/AvatarPerceiveObjActionGoalSubscriberCallback.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "Conversions.h"

FAvatarPerceiveObjActionGoalCallback::FAvatarPerceiveObjActionGoalCallback(
	const FString& InTopic, const FString& InType, UObject* InController) :
	FROSBridgeSubscriber(InTopic, InType)
{
	Controller = Cast<UAvatarPerceiveObjController>(InController);
}

void FAvatarPerceiveObjActionGoalCallback::Callback(TSharedPtr<FROSBridgeMsg> Msg)
{
	if (Controller)
	{
		TSharedPtr<urobosim_msgs::PerceiveObjectActionGoal> Command = StaticCastSharedPtr<urobosim_msgs::PerceiveObjectActionGoal>(Msg);
		actionlib_msgs::GoalID Id = Command->GetGoalId();
		Controller->GoalStatusList.Add(FGoalStatusInfo(Id.GetId(), Id.GetStamp().Secs, Id.GetStamp().NSecs));
		Controller->TypeToPerceive = Command->GetGoal().GetType();
		Controller->bActive = true;
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			Controller->PerceiveObject();
		}
		);

	}
}

TSharedPtr<FROSBridgeMsg> FAvatarPerceiveObjActionGoalCallback::ParseMessage
(TSharedPtr<FJsonObject> JsonObject) const
{

	TSharedPtr<urobosim_msgs::PerceiveObjectActionGoal> Message =
		MakeShareable<urobosim_msgs::PerceiveObjectActionGoal>(new urobosim_msgs::PerceiveObjectActionGoal());

	Message->FromJson(JsonObject);

	return StaticCastSharedPtr<FROSBridgeMsg>(Message);
}

FAvatarPerceiveObjActionGoalCallback::~FAvatarPerceiveObjActionGoalCallback()
{
}