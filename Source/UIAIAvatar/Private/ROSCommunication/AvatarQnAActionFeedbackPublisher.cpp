// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "ROSCommunication/AvatarQnAActionFeedbackPublisher.h"

/*
void UAvatarQnAActionFeedbackPublisher::SetMessageType()
{
	MessageType = TEXT("iai_avatar_msgs/QnAActionFeedback");
}


void UAvatarQnAActionFeedbackPublisher::SetOwner(UObject* InAgent)
{
	AActor* Agent = Cast<AActor>(InAgent);
	UAControllerComponent* ControllerComp = Agent->FindComponentByClass<UAControllerComponent>();
	Owner = Cast<UAvatarQnAController>(ControllerComp->Controller.ControllerList[ControllerName]);
}

void UAvatarQnAActionFeedbackPublisher::Publish(float DeltaTime)
{
	if (Owner->bActive && !skip(DeltaTime))
	{
		TSharedPtr<iai_avatar_msgs::QnAActionFeedback> Feedback =
			MakeShareable(new iai_avatar_msgs::QnAActionFeedback());

		FGoalStatusInfo StatusInfo = Owner->GoalStatusList.Last();
		actionlib_msgs::GoalStatus GS(actionlib_msgs::GoalID(FROSTime(StatusInfo.Secs, StatusInfo.NSecs), StatusInfo.Id), StatusInfo.Status, "");
		Feedback->SetStatus(GS);
		Feedback->SetHeader(std_msgs::Header(Seq, FROSTime(), ""));

		iai_avatar_msgs::QnAFeedback FeedbackMsg;

		Feedback->SetFeedback(FeedbackMsg);
		Handler->PublishMsg(Topic, Feedback);
		Handler->Process();

		Seq += 1;
	}
} */