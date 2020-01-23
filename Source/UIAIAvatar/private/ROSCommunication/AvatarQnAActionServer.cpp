// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "AvatarQnAActionServer.h"

UAvatarQnAActionServer::UAvatarQnAActionServer()
{
	CancelSubscriber = CreateDefaultSubobject<UAActionCancelSubscriber>(TEXT("QnAACancelSubscriber"));
	StatusPublisher = CreateDefaultSubobject<UAActionStatusPublisher>(TEXT("QnAAStatusPublisher"));
	ResultPublisher = CreateDefaultSubobject<UAvatarQnAActionResultPublisher>(TEXT("QnAAResultPublisher"));
	GoalSubscriber = CreateDefaultSubobject<UAvatarQnAActionGoalSubscriber>(TEXT("QnAAGoalSubscriber"));
	FeedbackPublisher = CreateDefaultSubobject<UAvatarQnAActionFeedbackPublisher>(TEXT("QnAAFeedbackPublisher"));

}