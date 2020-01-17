#pragma once

#include "ROSBridgeMsg.h"


namespace iai_avatar_msgs
{
	class QuestionResult : public FROSBridgeMsg
	{
		FString Answer;
	public:
		QuestionResult()
		{
			MsgType = "iai_avatar_msgs/QnAResult";
		}

		QuestionResult
		(
			FString InAnswer
		):
			Answer(InAnswer)
		{
			MsgType = "iai_avatar_msgs/QnAResult";
		}

		~QuestionResult() override {}

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

		static QuestionResult GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			QuestionResult Result;
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