#pragma once

#include "ROSBridgeSrv.h"

#include "geometry_msgs/Point.h"

namespace iai_avatar_msgs
{
	class PanTilt : public FROSBridgeSrv
	{

	public:
		PanTilt()
		{
			SrvType = TEXT("iai_avatar_msgs/PanTilt");
		}

		class Request : public SrvRequest
		{
		private:
			double Pan;
			double Tilt;

		public:
			Request() {}

			double GetPan() const
			{
				return Pan;
			}

			double GetTilt() const
			{
				return Tilt;
			}

			void SetPan(double InPan) 
			{
				Pan = InPan;
			}

			void SetTilt(double InTilt) 
			{
				Tilt = InTilt;
			}

			virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
			{
				UE_LOG(LogTemp, Error, TEXT("FromJson on Request"));

				Pan = JsonObject->GetNumberField(TEXT("pan"));
				Tilt = JsonObject->GetNumberField(TEXT("tilt"));
			}

			static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
			{
				Request Req;
				Req.FromJson(JsonObject);
				return Req;
			}

			virtual FString ToString() const override
			{
				return TEXT("PanTilt::Request { pan = ") + FString::SanitizeFloat(Pan) + TEXT(", tilt = ") + FString::SanitizeFloat(Tilt) + TEXT("} ");
			}

			virtual TSharedPtr<FJsonObject> ToJsonObject() const
			{
				TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
				Object->SetNumberField(TEXT("pan"), Pan);
				Object->SetNumberField(TEXT("tilt"), Tilt);
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



