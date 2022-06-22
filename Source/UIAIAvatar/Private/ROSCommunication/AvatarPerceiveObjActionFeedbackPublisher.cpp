// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "ROSCommunication/AvatarPerceiveObjActionFeedbackPublisher.h"

void UAvatarPerceiveObjActionFeedbackPublisher::SetMessageType()
{
	MessageType = TEXT("urobosim_msgs/PerceiveObjectActionFeedback");
}


void UAvatarPerceiveObjActionFeedbackPublisher::SetOwner(UObject* InAgent)
{
	AActor* Agent = Cast<AActor>(InAgent);
	UAControllerComponent* ControllerComp = Agent->FindComponentByClass<UAControllerComponent>();
	Owner = Cast<UAvatarPerceiveObjController>(ControllerComp->Controller.ControllerList[ControllerName]);
}

void UAvatarPerceiveObjActionFeedbackPublisher::Publish(float DeltaTime) {}