#pragma once

#include "ROSBridgeSrv.h"

#include "geometry_msgs/Vector3.h"

namespace iai_avatar_msgs
{
	class AvatarRotation : public FROSBridgeSrv
	{

	public:
		AvatarRotation()
		{
			SrvType = TEXT("iai_avatar_msgs/AvatarRotation");
		}

		class Request : public SrvRequest
		{
		private:
			geometry_msgs::Vector3 Rotation;

		public:
			Request() {}

			geometry_msgs::Vector3 GetRotation() const
			{
				return Rotation;
			}

			void SetRotation(geometry_msgs::Vector3 InRotation)
			{
				Rotation = InRotation;
			}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				UE_LOG(LogTemp, Error, TEXT("FromJson on Request"));

				Rotation = geometry_msgs::Vector3::GetFromJson(JsonObject->GetObjectField(TEXT("rotation")));
			}

			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}

			virtual FString ToString() const override
			{
				return TEXT("AvatarRotation::Request { rotation = ") + Rotation.ToString() + TEXT("} ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetObjectField(TEXT("rotation"), Rotation.ToJsonObject());
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
				return TEXT("PanTilt::Response { } ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				return Object;
			}
		};
	};
} // namespace iai_avatar_msgs 



