#pragma once

#include "ROSBridgeSrv.h"
#include "SemanticCamObj.h"
#include "geometry_msgs/Point.h"

namespace iai_avatar_msgs
{
	class SemanticCamera : public FROSBridgeSrv
	{

	public:
		SemanticCamera()
		{
			SrvType = TEXT("iai_avatar_msgs/SemanticCamera");
		}

		class Request : public SrvRequest
		{
		private:
			// Add your dfferent Request parameters here

		public:
			Request() {}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				
			}

			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}

			virtual FString ToString() const override
			{
				return TEXT("Command::Request { }");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				return Object;
			}
		};

		class Response : public SrvResponse
		{
		private:
			// Add your dfferent Response parameters here
			bool Success;
			FString Message;
			TArray<SemanticCamObj> Objects;

		public:
			Response() {
				Success = false;
			}

			Response(bool InSuccess) : Success(InSuccess) {}

			Response(bool InSuccess, FString InMsg) {
				Success = InSuccess;
				Message = InMsg;
			}

			Response(bool InSuccess, FString InMessage,	const TArray<SemanticCamObj>& InObjects)
			{
				Success = InSuccess;
				Message = InMessage;
				Objects = InObjects;
			}

			void SetSuccess(bool S)
			{
				Success = S;
			}

			bool GetSuccess()
			{
				return Success;
			}

			void SetMessage(FString S)
			{
				Message = S;
			}

			FString GetMessage()
			{
				return Message;
			}

			TArray<SemanticCamObj> Getobjects() const
			{
				return Objects;
			}

			void Setobjects(TArray<SemanticCamObj>& InObjects)
			{
				Objects = InObjects;
			}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				Success = JsonObject->GetBoolField(TEXT("success"));

				Message = JsonObject->GetStringField(TEXT("message"));

				TArray<TSharedPtr<FJsonValue>> ValuesPtrArr;

				Objects.Empty();
				ValuesPtrArr = JsonObject->GetArrayField(TEXT("objects"));
				for (auto &ptr : ValuesPtrArr)
					Objects.Add(SemanticCamObj::GetFromJson(ptr->AsObject()));

			}

			static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Response Result;
				Result.FromJson(JsonObject);
				return Result;
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const override
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

				Object->SetBoolField(TEXT("success"), Success);
				Object->SetStringField(TEXT("message"), Message);
				TArray<TSharedPtr<FJsonValue>> objectsArray;
				for (auto &val : Objects)
					objectsArray.Add(MakeShareable(new FJsonValueObject(val.ToJsonObject())));
				Object->SetArrayField(TEXT("objects"), objectsArray);
				return Object;
			}

			virtual FString ToString() const override
			{
				FString OutputString;
				OutputString = TEXT("Command::Response { success = ") + FString::FromInt((int)Success);
				OutputString += TEXT(", ") + Message + TEXT(", Objects {\n");
				for (auto &val : Objects)
					OutputString += TEXT("\t") + val.ToYamlString() + TEXT(",\n");
				OutputString += TEXT(" }} ");
				return OutputString;
			}

		};
	};
} // namespace iai_avatar_msgs 



