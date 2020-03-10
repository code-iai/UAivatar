#pragma once

#include "ROSBridgeMsg.h"


namespace iai_avatar_msgs
{
	class QnAResult : public FROSBridgeMsg
	{
		FString Answer;
	public:
		QnAResult()
		{
			MsgType = "iai_avatar_msgs/QnAResult";
		}

		QnAResult
		(
			FString InAnswer
		):
			Answer(InAnswer)
		{
			MsgType = "iai_avatar_msgs/QnAResult";
		}

		~QnAResult() override {}

		FString GetAnswer() const
		{
			return Answer;
		}

		void SetAnswer(FString InAnswer)
		{
			Answer = InAnswer;
		}

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
			Answer = JsonObject->GetStringField(TEXT("answer"));

		}

		static QnAResult GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			QnAResult Result;
			Result.FromJson(JsonObject);
			return Result;
		}

		virtual TSharedPtr<FJsonObject> ToJsonObject() const override
		{
			TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

			Object->SetStringField(TEXT("answer"), Answer);
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