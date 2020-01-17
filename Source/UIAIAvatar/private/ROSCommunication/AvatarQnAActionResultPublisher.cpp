// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "AvatarQnAActionResultPublisher.h"

void UAvatarQnAActionResultPublisher::SetMessageType()
{
	MessageType = TEXT("iai_avatar_msgs/QnAActionResult");
}

void UAvatarQnAActionResultPublisher::SetOwner(UObject* InAgent)
{
	AActor* Agent = Cast<AActor>(InAgent);
	UAControllerComponent* ControllerComp = Agent->FindComponentByClass<UAControllerComponent>();
	Owner = Cast<UAvatarQnAController>(ControllerComp->Controller.ControllerList[ControllerName]);
}

void UAvatarQnAActionResultPublisher::Publish()
{
	/* UE_LOG(LogTemp, Error, TEXT("bPublishResult pub: %s"), Owner->bPublishResult ? TEXT("True") : TEXT("False")); */
	if (Owner->bPublishResult)
	{
		UE_LOG(LogTemp, Error, TEXT("Publish QnA Result"));
		TSharedPtr<iai_avatar_msgs::QuestionActionResult> ActionResult =
			MakeShareable(new iai_avatar_msgs::QuestionActionResult());

		ActionResult->SetHeader(std_msgs::Header(Seq, FROSTime(), ""));

		FGoalStatusInfo StatusInfo = Owner->GoalStatusList.Last();
		actionlib_msgs::GoalStatus GS(actionlib_msgs::GoalID(FROSTime(StatusInfo.Secs, StatusInfo.NSecs), StatusInfo.Id), StatusInfo.Status, "");
		ActionResult->SetStatus(GS);

		iai_avatar_msgs::QuestionResult Result(Owner->Answer);
		ActionResult->SetResult(Result);

		Handler->PublishMsg(Topic, ActionResult);
		Handler->Process();

		Seq++;
		Owner->bPublishResult = false;
	}
}