#pragma once

#include "ROSBridgeMsg.h"


namespace iai_avatar_msgs
{
	class QnAFeedback : public FROSBridgeMsg
	{
	public:
		QnAFeedback()
		{
			MsgType = "iai_avatar_msgs/QnAFeedback";
		}

		~QnAFeedback() override {}

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
		}

		static QnAFeedback GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			QnAFeedback Result;
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