#pragma once

#include "ROSBridgeMsg.h"


namespace iai_avatar_msgs
{
	class QnAGoal : public FROSBridgeMsg
	{
		FString Question;
	public:
		QnAGoal()
		{
			MsgType = "iai_avatar_msgs/QnAGoal";
		}

		QnAGoal
		(
			FString InQuestion
		):
			Question(InQuestion)
		{
			MsgType = "iai_avatar_msgs/QnAGoal";
		}

		~QnAGoal() override {}

		FString GetQuestion() const
		{
			return Question;
		}

		void Setquestion(FString Inquestion)
		{
			Question = Inquestion;
		}

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
			Question = JsonObject->GetStringField(TEXT("question"));

		}

		static QnAGoal GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			QnAGoal Result;
			Result.FromJson(JsonObject);
			return Result;
		}

		virtual TSharedPtr<FJsonObject> ToJsonObject() const override
		{
			TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

			Object->SetStringField(TEXT("question"), Question);
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