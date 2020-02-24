#include "ROSCommunication/AvatarConsoleCommandServiceServer.h"

TSharedPtr<FROSBridgeSrv::SrvRequest> FAvatarConsoleCommandsServer::FromJson(TSharedPtr<FJsonObject> JsonObject) const
{
	TSharedPtr<iai_avatar_msgs::Command::Request> Request =
		MakeShareable(new iai_avatar_msgs::Command::Request());
	Request->FromJson(JsonObject);
	return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
}


TSharedPtr<FROSBridgeSrv::SrvResponse> FAvatarConsoleCommandsServer::Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest)
{
        TSharedPtr<iai_avatar_msgs::Command::Request> Request =
            StaticCastSharedPtr<iai_avatar_msgs::Command::Request>(InRequest);
	
	Owner->Command = Request->GetCommand();
	Owner->bSuccess = false;

	Owner->SendConsoleCommand();

	return MakeShareable<FROSBridgeSrv::SrvResponse>
		(new iai_avatar_msgs::Command::Response(Owner->bSuccess, Owner->Message));
}
