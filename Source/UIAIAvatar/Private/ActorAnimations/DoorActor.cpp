// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "ActorAnimations/DoorActor.h"
#include "Components/BoxComponent.h"

// Sets default values
ADoorActor::ADoorActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    //Create our Default Components
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
    DoorHandle_right = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorHandleMesh_right"));
    DoorHandle_left = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorHandleMesh_left"));
    Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorGlass = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorGlassMesh"));

    DoorTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimelineComp"));
    DoorHandleProxVolume_right = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorHandleProxVolume_right"));
    DoorHandleProxVolume_left = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorHandleProxVolume_left"));

    //Setup our Attachments
    DoorFrame->SetupAttachment(RootComponent);
    Door->AttachToComponent(DoorFrame, FAttachmentTransformRules::KeepRelativeTransform);
    DoorGlass->AttachToComponent(DoorFrame, FAttachmentTransformRules::KeepRelativeTransform);
    DoorHandle_right->AttachToComponent(DoorFrame, FAttachmentTransformRules::KeepRelativeTransform);
    DoorHandle_left->AttachToComponent(DoorFrame, FAttachmentTransformRules::KeepRelativeTransform);
    DoorHandleProxVolume_right->AttachToComponent(DoorHandle_right, FAttachmentTransformRules::KeepRelativeTransform);
    DoorHandleProxVolume_left->AttachToComponent(DoorHandle_left, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void ADoorActor::BeginPlay()
{
    Super::BeginPlay();

    UpdateFunctionFloat.BindDynamic(this, &ADoorActor::UpdateTimelineComp);

    //If we have a float curve, bind it's graph to our update function
    if (DoorTimelineFloatCurve)
    {
        DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
    }

    //Binding our Proximity Box Component to our Overlap Functions
    DoorHandleProxVolume_right->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnOverlapBegin);
    DoorHandleProxVolume_right->OnComponentEndOverlap.AddDynamic(this, &ADoorActor::OnOverlapEnd);

    DoorHandleProxVolume_left->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnOverlapBegin);
    DoorHandleProxVolume_left->OnComponentEndOverlap.AddDynamic(this, &ADoorActor::OnOverlapEnd);
}

void ADoorActor::UpdateTimelineComp(float Output)
{
    FRotator DoorNewRotation = FRotator(0.0f, Output, 0.f);
    Door->SetRelativeRotation(DoorNewRotation);
}

void ADoorActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    DoorTimelineComp->Play();
}

void ADoorActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    DoorTimelineComp->Reverse();
}

// Called every frame
void ADoorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
