#pragma once

#include "ROSBridgeMsg.h"


namespace iai_avatar_msgs
{
	class QuestionFeedback : public FROSBridgeMsg
	{
	public:
		QuestionFeedback()
		{
			MsgType = "iai_avatar_msgs/QnAFeedback";
		}

		~QuestionFeedback() override {}

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
		}

		static QuestionFeedback GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			QuestionFeedback Result;
			Result.FromJson(JsonObject);
			return Result;
		}

		virtual TSharedPtr<FJsonObject> ToJsonObject() const override
		{
			TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

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