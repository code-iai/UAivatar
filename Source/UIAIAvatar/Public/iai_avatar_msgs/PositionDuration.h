#pragma once

#include "ROSBridgeSrv.h"

#include "geometry_msgs/Vector3.h"

namespace iai_avatar_msgs
{
	class PositionDuration : public FROSBridgeSrv
	{

	public:
		PositionDuration()
		{
			SrvType = TEXT("iai_avatar_msgs/PositionDuration");
		}

		class Request : public SrvRequest
		{
		private:
			geometry_msgs::Vector3 Position;
			float Duration;
			

		public:
			Request() {}

			geometry_msgs::Vector3 GetPosition() const
			{
				return Position;
			}
			
			float GetDuration() const
			{
				return Duration;
			}

			void SetPosition(geometry_msgs::Vector3 InPosition)
			{
				Position = InPosition;
			}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				UE_LOG(LogTemp, Error, TEXT("FromJson on Request"));

				Position = geometry_msgs::Vector3::GetFromJson(JsonObject->GetObjectField(TEXT("position")));
				Duration = JsonObject->GetNumberField(TEXT("duration"));
			}

			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}

			virtual FString ToString() const override
			{
				return TEXT("PositionDuration::Request { rotation = ") + Position.ToString() + TEXT(", duration=") + FString::SanitizeFloat(Duration) + TEXT("}");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetObjectField(TEXT("position"), Position.ToJsonObject());
				Object->SetNumberField(TEXT("duration"), Duration);
				return Object;
			}
		};

		class Response : public SrvResponse
		{
		private:


		public:
			Response() {}


			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				return;
			}

			static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Response rep; rep.FromJson(JsonObject);
				return rep;
			}

			virtual FString ToString() const override
			{
				return TEXT("PositionDuration::Response { } ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				return Object;
			}
		};
	};
} // namespace iai_avatar_msgs 



