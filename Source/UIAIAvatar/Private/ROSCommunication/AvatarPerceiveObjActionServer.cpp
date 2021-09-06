// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#include "ROSCommunication/AvatarPerceiveObjActionServer.h"

UAvatarPerceiveObjActionServer::UAvatarPerceiveObjActionServer()
{
	CancelSubscriber = CreateDefaultSubobject<UROSActionCancelSubscriber>(TEXT("GCACancelSubscriber"));
	StatusPublisher = CreateDefaultSubobject<UROSActionStatusPublisher>(TEXT("POAStatusPublisher"));
	ResultPublisher = CreateDefaultSubobject<UAvatarPerceiveObjActionResultPublisher>(TEXT("POAResultPublisher"));
	GoalSubscriber = CreateDefaultSubobject<UAvatarPerceiveObjActionGoalSubscriber>(TEXT("POAGoalSubscriber"));
	FeedbackPublisher = CreateDefaultSubobject<UAvatarPerceiveObjActionFeedbackPublisher>(TEXT("POAFeedbackPublisher"));
}