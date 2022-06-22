#include "ROSCommunication/AvatarSemanticCameraServiceServer.h"

TSharedPtr<FROSBridgeSrv::SrvRequest> FAvatarSemanticCameraServer::FromJson(TSharedPtr<FJsonObject> JsonObject) const
{
	TSharedPtr<iai_avatar_msgs::SemanticCamera::Request> Request =
		MakeShareable(new iai_avatar_msgs::SemanticCamera::Request());
	Request->FromJson(JsonObject);
	return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
}


TSharedPtr<FROSBridgeSrv::SrvResponse> FAvatarSemanticCameraServer::Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest)
{
	TSharedPtr<iai_avatar_msgs::SemanticCamera::Request> Request =
		StaticCastSharedPtr<iai_avatar_msgs::SemanticCamera::Request>(InRequest);
		
	Owner->ListObjects();

	TArray<iai_avatar_msgs::SemanticCamObj> Objects;
	
	for (auto &Item : Owner->Objects) {
		iai_avatar_msgs::SemanticCamObj Object(Item.Name, Item.Tags);
		Objects.Add(Object);
	}

	return MakeShareable<FROSBridgeSrv::SrvResponse>
		(new iai_avatar_msgs::SemanticCamera::Response(true,Owner->Message, Objects));
}
