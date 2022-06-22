// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/AvatarPerceiveObjActionResultPublisher.h"

void UAvatarPerceiveObjActionResultPublisher::SetMessageType()
{
	MessageType = TEXT("urobosim_msgs/PerceiveObjectActionResult");
}

void UAvatarPerceiveObjActionResultPublisher::SetOwner(UObject* InAgent)
{
	AActor* Agent = Cast<AActor>(InAgent);
	UAControllerComponent* ControllerComp = Agent->FindComponentByClass<UAControllerComponent>();
	Owner = Cast<UAvatarPerceiveObjController>(ControllerComp->Controller.ControllerList[ControllerName]);
}

void UAvatarPerceiveObjActionResultPublisher::Publish()
{
	if (Owner->bPublishResult)
	{
		UE_LOG(LogTemp, Error, TEXT("Publish Result"));
		TSharedPtr<urobosim_msgs::PerceiveObjectActionResult> ActionResult =
			MakeShareable(new urobosim_msgs::PerceiveObjectActionResult());

		ActionResult->SetHeader(std_msgs::Header(Seq, FROSTime(), ""));

		// uint8 status = Owner->Status;
		FGoalStatusInfo StatusInfo = Owner->GoalStatusList.Last();
		actionlib_msgs::GoalStatus GS(actionlib_msgs::GoalID(FROSTime(StatusInfo.Secs, StatusInfo.NSecs), StatusInfo.Id), StatusInfo.Status, "");
		ActionResult->SetStatus(GS);

		// In Base Footprint
		geometry_msgs::Point Point(FConversions::UToROS(Owner->ObjectPose.GetLocation()));
		geometry_msgs::Quaternion Orientation(FConversions::UToROS(Owner->ObjectPose.GetRotation()));

		geometry_msgs::Pose Pose(Point, Orientation);
		geometry_msgs::PoseStamped PoseStamped(std_msgs::Header(Seq, FROSTime(), "base_footprint"), Pose);

		// In World coordinates
		geometry_msgs::Point PointWorld(FConversions::UToROS(Owner->ObjectPoseWorld.GetLocation()));
		geometry_msgs::Quaternion OrientationWorld(FConversions::UToROS(Owner->ObjectPoseWorld.GetRotation()));

		geometry_msgs::Pose PoseWorld(PointWorld, OrientationWorld);
		geometry_msgs::PoseStamped PoseWorldStamped(std_msgs::Header(Seq, FROSTime(), "map"), PoseWorld);


		urobosim_msgs::PerceiveObjectResult Result(Owner->ObjectName, PoseStamped, PoseWorldStamped);
		ActionResult->SetResult(Result);

		Handler->PublishMsg(Topic, ActionResult);
		Handler->Process();

		Seq++;
		Owner->bPublishResult = false;

	}
}