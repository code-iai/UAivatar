// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#include "IAIAvatarROSCommandBase.h"
#include "IAIAvatarROSIntegrationComponent.h"


// Sets default values for this component's properties
UIAIAvatarROSCommandBase::UIAIAvatarROSCommandBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIAIAvatarROSCommandBase::BeginPlay()
{
	Super::BeginPlay();

	// Bind to the SendCommand Delegate of the UIAIAvatarROSIntegrationComponent
	if (UIAIAvatarROSIntegrationComponent* ResponsibleROSIntegrationComponent
		= GetOwner()->FindComponentByClass<UIAIAvatarROSIntegrationComponent>()) {
		ResponsibleROSIntegrationComponent->ROSSendCommandRequest.AddDynamic(this, &UIAIAvatarROSCommandBase::OnROSSendCommandRequest);
		UE_LOG(LogAvatarROS, Log,
			TEXT("Bound SendCommand Delegate to UIAIAvatarROSCommandLiftArm::OnROSSendCommandRequest on %s."), *(GetOwner()->GetName())
		);
	}
	else {
		UE_LOG(LogAvatarROS, Error,
			TEXT("Can't bind to ROSIntegrationComponent. Couldn't find a UIAIAvatarROSIntegrationComponent on %s. Maybe you've not added a UIAIAvatarROSIntegrationComponent to the Actor."), *(GetOwner()->GetName())
		);
	}

	// ...
	
}


// Called every frame
void UIAIAvatarROSCommandBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIAIAvatarROSCommandBase::OnROSSendCommandRequest(FString command) {
	UE_LOG(LogAvatarROS, Error, TEXT("OnROSSendCommandRequest received in UIAIAvatarROSCommandBase"));
}