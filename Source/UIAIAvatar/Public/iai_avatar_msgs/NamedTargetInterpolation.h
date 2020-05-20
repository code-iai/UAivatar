#pragma once

#include "ROSBridgeSrv.h"
#include "geometry_msgs/Vector3.h"

namespace iai_avatar_msgs
{
	class NamedTargetInterpolation : public FROSBridgeSrv
	{

	public:
		NamedTargetInterpolation()
		{
			SrvType = TEXT("iai_avatar_msgs/NamedTargetInterpolation");
		}

		class Request : public SrvRequest
		{
		private:
			FString Name;
			geometry_msgs::Vector3 Target;
			float Duration;
			

		public:
			Request() {}

			FString GetName() const
			{
				return Name;
			}
			
			geometry_msgs::Vector3 GetTarget() const
			{
				return Target;
			}
			
			float GetDuration() const
			{
				return Duration;
			}

			void SetName(FString InName)
			{
				Name = InName;
			}
			
			void SetTarget(geometry_msgs::Vector3 InTarget)
			{
				Target = InTarget;
			}

			void SetDuration(float InDuration) 
			{
				Duration = InDuration;
			}
			
			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				UE_LOG(LogTemp, Error, TEXT("FromJson on Request"));

				Name = JsonObject->GetStringField(TEXT("name"));
				Target = geometry_msgs::Vector3::GetFromJson(JsonObject->GetObjectField(TEXT("target")));
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
				return TEXT("NamedTargetInterpolation::Request { name=") + Name + TEXT(", target = ") + Target.ToString() + TEXT(", duration=") + FString::SanitizeFloat(Duration) + TEXT("}");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetStringField(TEXT("name"), Name);
				Object->SetObjectField(TEXT("target"), Target.ToJsonObject());
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
				return TEXT("NamedTargetInterpolation::Response { } ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				return Object;
			}
		};
	};
} // namespace iai_avatar_msgs 



