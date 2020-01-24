// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "AActionStatusPublisher.h"

void UAActionStatusPublisher::SetMessageType()
{
	MessageType = TEXT("actionlib_msgs/GoalStatusArray");
}

void UAActionStatusPublisher::Publish(float DeltaTime)
{
	if (!skip(DeltaTime)) {
		
		TSharedPtr<actionlib_msgs::GoalStatusArray> GoalStatusArrayMsg =
			MakeShareable(new actionlib_msgs::GoalStatusArray());

		GoalStatusArrayMsg->SetHeader(std_msgs::Header(Seq, FROSTime(), ""));

		TArray<actionlib_msgs::GoalStatus> GSArray;

		for (auto& StatusInfo : Owner->GoalStatusList)
		{
			actionlib_msgs::GoalStatus GS(actionlib_msgs::GoalID(FROSTime(StatusInfo.Secs, StatusInfo.NSecs), StatusInfo.Id), StatusInfo.Status, "");
			GSArray.Add(GS);
		}
		GoalStatusArrayMsg->SetStatusList(GSArray);

		Handler->PublishMsg(Topic, GoalStatusArrayMsg);
		Handler->Process();

		Seq++;
	}
}

void UAActionStatusPublisher::SetOwner(UObject* InAgent)
{
	AActor* Agent = Cast<AActor>(InAgent);
	UAControllerComponent* ControllerComp = Agent->FindComponentByClass<UAControllerComponent>();
	Owner = Cast<UAController>(ControllerComp->Controller.ControllerList[ControllerName]);
}