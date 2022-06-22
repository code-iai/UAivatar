#pragma once

#include "ROSBridgeSrv.h"

#include "geometry_msgs/Point.h"

namespace iai_avatar_msgs
{
	class Command : public FROSBridgeSrv
	{

	public:
		Command()
		{
			SrvType = TEXT("iai_avatar_msgs/Command");
		}

		class Request : public SrvRequest
		{
		private:
			// Add your dfferent Request parameters here
			FString Command;

		public:
			Request() {}

			FString GetCommand() const
			{
				return Command;
			}

			void SetCommand(FString InCommand) 
			{
				Command = InCommand;
			}


			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				Command = JsonObject->GetStringField(TEXT("command"));
			}

			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}

			virtual FString ToString() const override
			{
				return TEXT("Command::Request { command = ") + Command + TEXT("} ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetStringField(TEXT("command"), Command);
				return Object;
			}
		};

		class Response : public SrvResponse
		{
		private:
			// Add your dfferent Response parameters here
			bool Success;
			FString Message;

		public:
			Response() {
				Success = false;
			}

			Response(bool InSuccess) : Success(InSuccess) {}

			Response(bool InSuccess, FString InMsg) {
				Success = InSuccess;
				Message = InMsg;
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

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				Success = JsonObject->GetBoolField("success");
				Message = JsonObject->GetStringField("message");
			}

			static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Response rep; 
				rep.FromJson(JsonObject);
				return rep;
			}

			virtual FString ToString() const override
			{
				return TEXT("Command::Response { success = ") + FString::FromInt( (int)Success) + TEXT(", ") + Message + TEXT(" } ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetBoolField("success", Success);
				Object->SetStringField("message", Message);
				return Object;
			}
		};
	};
} // namespace iai_avatar_msgs 



