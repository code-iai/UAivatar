#pragma once

#include "ROSBridgeSrv.h"


namespace iai_avatar_msgs
{
	class GeneralGrasp : public FROSBridgeSrv
	{
	public:
		GeneralGrasp()
		{
			SrvType = TEXT("iai_avatar_msgs/GeneralGrasp");
		}

		class Request : public SrvRequest
		{
		private:
			FString Command;
			FString Targetobject;
					
		public:
			Request(){ }
			Request(FString InCommand,
				FString InTargetobject)
				:
				Command(InCommand),
				Targetobject(InTargetobject) { }
			
			
			// Getters 
			FString GetCommand() const { return Command; }
			FString GetTargetobject() const { return Targetobject; }
			
			
			// Setters 
			void SetCommand(FString InCommand) { Command = InCommand; }
			void SetTargetobject(FString InTargetobject) { Targetobject = InTargetobject; }
			
			
			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				Command = JsonObject->GetStringField(TEXT("command"));

				Targetobject = JsonObject->GetStringField(TEXT("targetObject"));

			}
			
			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}
			
//			### TOSTRING ###
			
			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

				Object->SetStringField(TEXT("command"), Command);

				Object->SetStringField(TEXT("targetObject"), Targetobject);

				return Object;

			}
		};
		
		class Response : public SrvResponse
		{
		private:
			bool Success;
			TArray<FString> List;
			
			
		public:
			Response(){ }
			Response(bool InSuccess,
				TArray<FString> InList)
				:
				Success(InSuccess),
				List(InList) { }
			
			
			// Getters 
			bool GetSuccess() const { return Success; }
			TArray<FString> GetList() const { return List; }
			
			
			// Setters 
			void SetSuccess(bool InSuccess) { Success = InSuccess; }
			void SetList(TArray<FString> InList) { List = InList; }
			
			
			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				TArray<TSharedPtr<FJsonValue>> ValuesPtrArr;

				Success = JsonObject->GetBoolField(TEXT("success"));

				List.Empty();
				ValuesPtrArr = JsonObject->GetArrayField(TEXT("list"));
				for (auto &ptr : ValuesPtrArr)
					List.Add(ptr->AsString());

			}
			
			static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Response Resp; 
				Resp.FromJson(JsonObject);
				return Resp;
			}			
			
//			### TOSTRING ###
			
			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());

				Object->SetBoolField(TEXT("success"), Success);

				TArray<TSharedPtr<FJsonValue>> ListArray;
				for (auto &val : List)
					ListArray.Add(MakeShareable(new FJsonValueString(val)));
				Object->SetArrayField(TEXT("list"), ListArray);

				return Object;

			}
		};
		
	};
	
}