#pragma once

#include "ROSBridgeMsg.h"


namespace iai_avatar_msgs
{
	class SemanticCamObj : public FROSBridgeMsg
	{
		FString Name;
		TArray<FString> Tags;
	public:
		SemanticCamObj()
		{
			MsgType = "iai_avatar_msgs/SemanticCamObj";
		}

		SemanticCamObj
		(
			FString InName,
			const TArray<FString>& InTags
		):
			Name(InName),
			Tags(InTags)
		{
			MsgType = "iai_avatar_msgs/SemanticCamObj";
		}

		~SemanticCamObj() override {}

		FString GetName() const
		{
			return Name;
		}

		TArray<FString> GetTags() const
		{
			return Tags;
		}

		void SetName(FString InName)
		{
			Name = InName;
		}

		void SetTag(TArray<FString>& InTags)
		{
			Tags = InTags;
		}

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
			Name = JsonObject->GetStringField(TEXT("name"));
			TArray<TSharedPtr<FJsonValue>> ValuePtrArr;

			Tags.Empty();
			ValuePtrArr = JsonObject->GetArrayField(TEXT("tags"));
			for (auto &ptr : ValuePtrArr)
				Tags.Add(ptr->AsString());

		}

		static SemanticCamObj GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
			SemanticCamObj Result;
			Result.FromJson(JsonObject);
			return Result;
		}

		virtual TSharedPtr<FJsonObject> ToJsonObject() const override
		{
			TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

			Object->SetStringField(TEXT("name"), Name);
			
			TArray<TSharedPtr<FJsonValue>> TagsArray;
			for (auto &val : Tags)
				TagsArray.Add(MakeShareable(new FJsonValueString(val)));

			Object->SetArrayField(TEXT("tags"), TagsArray);
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