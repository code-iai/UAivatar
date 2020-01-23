#pragma once

#include "ROSBridgeMsg.h"

#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "iai_avatar_msgs/actions/QnAFeedback.h"

namespace iai_avatar_msgs
{
	class QnAActionFeedback : public FROSBridgeMsg
	{
		std_msgs::Header Header;
		actionlib_msgs::GoalStatus Status;
		QnAFeedback Feedback;
	public:
		QnAActionFeedback()
		{
			MsgType = "iai_avatar_msgs/QnAActionFeedback";
		}

		QnAActionFeedback
		(
			std_msgs::Header InHeader,
			actionlib_msgs::GoalStatus InStatus,
			QnAFeedback InFeedback
		):
			Header(InHeader),
			Status(InStatus),
			Feedback(InFeedback)
		{
			MsgType = "iai_avatar_msgs/QnAActionFeedback";
		}

		~QnAActionFeedback() override {}

		std_msgs::Header GetHeader() const
		{
			return Header;
		}

		actionlib_msgs::GoalStatus GetStatus() const
		{
			return Status;
		}

		QnAFeedback GetFeedback() const
		{
			return Feedback;
		}

		void SetHeader(std_msgs::Header InHeader)
		{
			Header = InHeader;
		}

		void SetStatus(actionlib_msgs::GoalStatus InStatus)
		{
			Status = InStatus;
		}

		void SetFeedback(QnAFeedback Infeedback)
		{
			Feedback = Infeedback;
		}

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
			Header = std_msgs::Header::GetFromJson(JsonObject->GetObjectField(TEXT("header")));

			Status = actionlib_msgs::GoalStatus::GetFromJson(JsonObject->GetObjectField(TEXT("status")));

			Feedback = QnAFeedback::GetFromJson(JsonObject->GetObjectField(TEXT("feedback")));

		}

		static QnAActionFeedback GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			QnAActionFeedback Result;
			Result.FromJson(JsonObject);
			return Result;
		}

		virtual TSharedPtr<FJsonObject> ToJsonObject() const override
		{
			TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

			Object->SetObjectField(TEXT("header"), Header.ToJsonObject());
			Object->SetObjectField(TEXT("status"), Status.ToJsonObject());
			Object->SetObjectField(TEXT("feedback"), Feedback.ToJsonObject());
			return Object;
		}
		virtual FString ToYamlString() const override
		{
			FString OutputString;
			TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(ToJsonObject().ToSharedRef(), Writer);
			return OutputString;
		}
	};
}