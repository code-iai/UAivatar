#include "ROSCommunication/AvatarActionCancelSubscriberCallback.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "Conversions.h"

FAvatarActionCancelCallback::FAvatarActionCancelCallback(
	const FString& InTopic, const FString& InType, UObject* InController) :
	FROSBridgeSubscriber(InTopic, InType)
{
	Controller = Cast<UAController>(InController);
}

void FAvatarActionCancelCallback::Callback(TSharedPtr<FROSBridgeMsg> Msg)
{
	if (Controller)
	{
		Controller->bCancel = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Action Controller not found"));
	}
}

TSharedPtr<FROSBridgeMsg> FAvatarActionCancelCallback::ParseMessage
(TSharedPtr<FJsonObject> JsonObject) const
{
	TSharedPtr<actionlib_msgs::GoalID> GoalId =
		MakeShareable<actionlib_msgs::GoalID>(new actionlib_msgs::GoalID());

	GoalId->FromJson(JsonObject);

	return StaticCastSharedPtr<FROSBridgeMsg>(GoalId);
}

FAvatarActionCancelCallback::~FAvatarActionCancelCallback()
{
}