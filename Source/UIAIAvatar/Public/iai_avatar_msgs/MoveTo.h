#pragma once

#include "ROSBridgeSrv.h"

#include "geometry_msgs/Point.h"

namespace iai_avatar_msgs
{
	class MoveTo : public FROSBridgeSrv
	{

	public:

		MoveTo()
		{
			SrvType = TEXT("iai_avatar_msgs/MoveTo");
		}

		class Request : public SrvRequest
		{
		private:
			geometry_msgs::Point Position;

		public:
			Request() {}

			geometry_msgs::Point GetPosition() const
			{
				return Position;
			}

			void SetPosition(geometry_msgs::Point InPosition)
			{
				Position = InPosition;
			}

			//int64 GetPosition() const 
			//{ 
			//	return Position; 
			//}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				UE_LOG(LogTemp, Error, TEXT("FromJson on Request"));

				Position = geometry_msgs::Point::GetFromJson(JsonObject->GetObjectField(TEXT("position")));
			}

			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}

			virtual FString ToString() const override
			{
				return TEXT("MoveTo::Request { position = ") + Position.ToString() + TEXT("} ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetObjectField(TEXT("position"), Position.ToJsonObject());
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
				return TEXT("MoveTo::Response { } ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				return Object;
			}
		};
	};
} // namespace iai_avatar_msgs 
