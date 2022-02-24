// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "BDI/Patrol.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "IAIAvatar_AIController.h"
#include "IAIAvatarCharacter.h"
//#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BDI/BlackBoardKeys.h"


UPatrol::UPatrol(FObjectInitializer const& object_initializer)
{
	NodeName = TEXT("Patrol");
}

EBTNodeResult::Type UPatrol::ExecuteTask(UBehaviorTreeComponent& owner_comp, uint8* node_memory)
{
	auto const Controller = Cast<AIAIAvatar_AIController>(owner_comp.GetAIOwner());
	auto const Character = Cast<AIAIAvatarCharacter>(Controller->GetCharacter());

	
	Character->RandomPatrol();

	// finish with success
	FinishLatentTask(owner_comp, EBTNodeResult::Succeeded);
	return EBTNodeResult::Succeeded;
}
