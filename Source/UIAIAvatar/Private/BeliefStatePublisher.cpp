// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "BeliefStatePublisher.h"
#include "ROSBridgeGameInstance.h"
#include "UIAIAvatarModule.h" // include for log levels
#include "IAIAvatarCharacter.h"
#include "IAIAvatarAnimationInstance.h"
#include "Conversions.h"
//#include "std_msgs/Int32.h"

// Called when the game starts


UBeliefStatePublisher::UBeliefStatePublisher()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	CurrentDeltaForPublishRate = 0;
}
//::SetMessageType()
//{
//	MessageType = TEXT("std_msgs/Int32");
//}
//
//void UBeliefStatePublisher::SetBelief(UObject* InAgent)
//{
//	
//}
bool UBeliefStatePublisher::GetValue() 
{
    return bShelfIsEmpty;
}

void UBeliefStatePublisher::SetValue()
{
    bShelfIsEmpty = true;
}

void UBeliefStatePublisher::BeginPlay()
{
    Super::BeginPlay();
    


    // Create the ROSBridge handler for connecting with ROS
    //ROSBridgeHandler = MakeShareable<FROSBridgeHandler>(
           // new FROSBridgeHandler(ServerIP, ServerPORT));

    // Create the tf publisher
    BFSPublisher = MakeShareable<FROSBridgePublisher>(
        new FROSBridgePublisher("bfs", "std_msgs/Int32"));

    // Getting the actual UROSBridgeGameInstance
    UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetOwner()->GetGameInstance());
    check(ActiveGameInstance);

    if (!ActiveGameInstance->ROSHandler.IsValid() || !ActiveGameInstance->ROSHandler->IsClientConnected())
    {
        UE_LOG(LogAvatarROS, Error, TEXT("Not connected to ROS. Can't initialize IAIAvatarROSIntegrationComponent on %s"), *(GetOwner()->GetName()));
        return;
    }

    ActiveGameInstance->ROSHandler->AddPublisher(BFSPublisher);
    //TFPublisher = MakeShareable<FROSBridgePublisher>(
    //	new FROSBridgePublisher("tf_static", "tf2_msgs/TFMessage"));

    // Connect to ROS
    //ROSBridgeHandler->Connect();

    // Add publisher
    //ROSBridgeHandler->AddPublisher(TFPublisher);


}

void UBeliefStatePublisher::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    CurrentDeltaForPublishRate += DeltaTime;
    if (CurrentDeltaForPublishRate < ConstantPublishRate)
        return;

    // We've exceeded deltatime. Reset counter and execute tf
    CurrentDeltaForPublishRate = 0;

    FROSTime TimeNow = FROSTime::Now();

    UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetOwner()->GetGameInstance());
    check(ActiveGameInstance);

    if (!ActiveGameInstance->ROSHandler.IsValid() || !ActiveGameInstance->ROSHandler->IsClientConnected())
    {
        UE_LOG(LogAvatarROS, Error, TEXT("Not connected to ROS. Can't initialize IAIAvatarROSIntegrationComponent on %s"), *(GetOwner()->GetName()));
        return;
    }
    TSharedPtr<std_msgs::Int32> BeliefState =
        MakeShareable(new std_msgs::Int32());

    if(bShelfIsEmpty)
    {
    BeliefState->SetData(1);
    UE_LOG(LogTemp, Error, TEXT("Shelf is Empty"));
    }


    ActiveGameInstance->ROSHandler->PublishMsg("/bfs", BeliefState);
    ActiveGameInstance->ROSHandler->Process();
      
    // Update message sequence
    Seq++;
 }

//void UBeliefStatePublisher::Publish()
//{
//    FROSTime TimeNow = FROSTime::Now();
//
//    UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetOwner()->GetGameInstance());
//    check(ActiveGameInstance);
//
//    if (!ActiveGameInstance->ROSHandler.IsValid() || !ActiveGameInstance->ROSHandler->IsClientConnected())
//    {
//        UE_LOG(LogAvatarROS, Error, TEXT("Not connected to ROS. Can't initialize IAIAvatarROSIntegrationComponent on %s"), *(GetOwner()->GetName()));
//        return;
//    }
//
//    TSharedPtr<std_msgs::Int32> BeliefState =
//        MakeShareable(new std_msgs::Int32());
//
//    //std_msgs::Int32 BeliefState;
//
//    BeliefState->SetData(1);
//
//
//    UE_LOG(LogTemp, Error, TEXT("Shelf is Empty"));
//
//
//    ActiveGameInstance->ROSHandler->PublishMsg("/bfs", BeliefState);
//    ActiveGameInstance->ROSHandler->Process();
//}

//{
//	
//		TSharedPtr<std_msgs::Int32> BeliefState =
//			MakeShareable(new std_msgs::Int32());
//
//		//std_msgs::Int32 BeliefState;
//
//		BeliefState->SetData(1);
//
//		//UROSPublisher* ROSPublisher = new UROSPublisher();
//
//		Handler->PublishMsg("std_msgs/Int32", BeliefState);
//		Handler->Process();
//
//		Seq++;
//	
//
//		UE_LOG(LogTemp, Error, TEXT("Shelf is Empty"));
//	
//}

