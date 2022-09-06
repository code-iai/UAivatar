// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/AvatarQnAActionResultPublisher.h"

/*
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
	if (Owner->bPublishResult)
	{
		UE_LOG(LogTemp, Log, TEXT("Publish QnA Result: %s"), *Owner->Answer);
		TSharedPtr<iai_avatar_msgs::QnAActionResult> ActionResult =
			MakeShareable(new iai_avatar_msgs::QnAActionResult());

		ActionResult->SetHeader(std_msgs::Header(Seq, FROSTime(), ""));

		FGoalStatusInfo StatusInfo = Owner->GoalStatusList.Last();
		actionlib_msgs::GoalStatus GS(actionlib_msgs::GoalID(FROSTime(StatusInfo.Secs, StatusInfo.NSecs), StatusInfo.Id), StatusInfo.Status, "");
		ActionResult->SetStatus(GS);

		iai_avatar_msgs::QnAResult Result(Owner->Answer);
		ActionResult->SetResult(Result);

		Handler->PublishMsg(Topic, ActionResult);
		Handler->Process();

		Seq++;
		Owner->bPublishResult = false;

		UE_LOG(LogTemp, Error, TEXT("Publish QnA Result Sent"));
	}
}
*/