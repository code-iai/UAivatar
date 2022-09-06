#include "ROSCommunication/AvatarQnAActionGoalSubscriberCallback.h"
#include "actionlib_msgs/GoalStatusArray.h"
#include "Conversions.h"
/*
FAvatarQnAActionGoalCallback::FAvatarQnAActionGoalCallback(
	const FString& InTopic, const FString& InType, UObject* InController) :
	FROSBridgeSubscriber(InTopic, InType)
{
	Controller = Cast<UAvatarQnAController>(InController);
}

void FAvatarQnAActionGoalCallback::Callback(TSharedPtr<FROSBridgeMsg> Msg)
{
	if (Controller)
	{
		TSharedPtr<iai_avatar_msgs::QnAActionGoal> QuestionMSg =
			StaticCastSharedPtr<iai_avatar_msgs::QnAActionGoal>(Msg);
		Controller->Question = QuestionMSg->GetGoal().GetQuestion();

		actionlib_msgs::GoalID Id = QuestionMSg->GetGoalId();
		
		Controller->GoalStatusList.Add(FGoalStatusInfo(Id.GetId(), Id.GetStamp().Secs, Id.GetStamp().NSecs));
		UE_LOG(LogTemp, Log, TEXT("Recieved QnA Action Id: %s Question: %s"), *Id.GetId(), *Controller->Question);
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			Controller->NotifyQuestion();
		}
		);
	}
}

TSharedPtr<FROSBridgeMsg> FAvatarQnAActionGoalCallback::ParseMessage
(TSharedPtr<FJsonObject> JsonObject) const
{
	TSharedPtr<iai_avatar_msgs::QnAActionGoal> Message =
		MakeShareable<iai_avatar_msgs::QnAActionGoal>(new iai_avatar_msgs::QnAActionGoal());

	Message->FromJson(JsonObject);

	return StaticCastSharedPtr<FROSBridgeMsg>(Message);
}

FAvatarQnAActionGoalCallback::~FAvatarQnAActionGoalCallback()
{
}*/