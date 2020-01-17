// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "AvatarQnAActionServer.h"

UAvatarQnAActionServer::UAvatarQnAActionServer()
{
	CancelSubscriber = CreateDefaultSubobject<UAvatarActionCancelSubscriber>(TEXT("QnAACancelSubscriber"));
	StatusPublisher = CreateDefaultSubobject<UAPublisher>(TEXT("QnAAStatusPublisher"));
	ResultPublisher = CreateDefaultSubobject<UAvatarQnAActionResultPublisher>(TEXT("QnAAResultPublisher"));
	GoalSubscriber = CreateDefaultSubobject<UAvatarQnAActionGoalSubscriber>(TEXT("QnAAGoalSubscriber"));
	FeedbackPublisher = CreateDefaultSubobject<UAPublisher>(TEXT("QnAAFeedbackPublisher"));

}