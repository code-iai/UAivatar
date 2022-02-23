// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen


#include "BDI/IAIAvatar_AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "IAIAvatarCharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "BDI/BlackBoardKeys.h"
#include "BDI/ai_tags.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "EnvironmentQuery/EnvQueryManager.h"

AIAIAvatar_AIController::AIAIAvatar_AIController(FObjectInitializer const& object_initializer)
{
	//todo: select different BT
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> obj(TEXT("BehaviorTree'/UIAIAvatar/AI-BT/SK_BT_1.SK_BT_1'"));

	if (obj.Succeeded())
	{
		BehaviorTree = obj.Object;
	}

	BehaviorTreeComp = object_initializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorComp"));

	BlackboardComp = object_initializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackboardComp"));

	SetupPerceptionSystem(); 
}

void AIAIAvatar_AIController::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTree);

	BehaviorTreeComp->StartTree(*BehaviorTree);

}

void AIAIAvatar_AIController::OnPossess(APawn* const pawn)
{
	Super::OnPossess(pawn);
	if (BlackboardComp)
	{
		BlackboardComp->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	}
}

UBlackboardComponent* AIAIAvatar_AIController::GetBlackboard() const
{
	return BlackboardComp;
}

void AIAIAvatar_AIController::OnPerceptionUpdated(TArray<AActor*> const& UpdatedActors)
{
	//If Avatar detects EmptyShelf register this to BlackboardComponent

	for (size_t x = 0; x < UpdatedActors.Num(); ++x)
	{
		FActorPerceptionBlueprintInfo Info;

		GetPerceptionComponent()->GetActorsPerception(UpdatedActors[x], Info);
		
		for (size_t k = 0; k < Info.LastSensedStimuli.Num(); ++k)
		{
			FAIStimulus const stim = Info.LastSensedStimuli[k];

			if (stim.Tag == tags::EmptyShelfTag)
			{
				GetBlackboard()->SetValueAsBool(BBKeys::shelf_is_empty, stim.WasSuccessfullySensed());
				//GetBlackboard()->SetValueAsVector(BBKeys::target_location, stim.StimulusLocation);
			}

			// if (UpdatedActors[x]-> has IsA<AAICharacter_AD>() && !GetSeeingPawn())
			//{
			//	//BlackboardComp->SetValueAsObject(BlackboardKey_EmptyShelf, UpdatedActors[x]);
			//	GetBlackboard()->SetValueAsBool(BBKeys::shelf_is_empty, stim.WasSuccessfullySensed());
			//	return;
			//}
			/*else if (stim.Type.Name == "Default__AISense_Sight")
			{
				get_blackboard()->SetValueAsBool(bb_keys::can_see_player, stim.WasSuccessfullySensed());
			}*/
		}
	}
}

void AIAIAvatar_AIController::SetupPerceptionSystem()
{
	// create and initialise sight configuration object
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (SightConfig)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
		SightConfig->SightRadius = 500.0f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 25.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		// add sight configuration component to perception component
		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}

	//// create and initialise hearing config object
	//hearing_config = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing config"));
	//if (hearing_config)
	//{
	//	hearing_config->HearingRange = 3000.0f;
	//	hearing_config->DetectionByAffiliation.bDetectEnemies =
	//		hearing_config->DetectionByAffiliation.bDetectFriendlies =
	//		hearing_config->DetectionByAffiliation.bDetectNeutrals = true;

	//	// add sight configuration component to perception component
	//	GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AIAIAvatar_AIController::OnPerceptionUpdated);
	//	GetPerceptionComponent()->ConfigureSense(*hearing_config);
	//}
}


//void AIAIAvatar_AIController::FindHidingSpot()
//{
//	FEnvQueryRequest HidingSpotQueryRequest = FEnvQueryRequest(FindHidingSpotEQS, GetPawn());
//	HidingSpotQueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &AIAIAvatar_AIController::MoveToQueryResult);
//}
//
//void AIAIAvatar_AIController::MoveToQueryResult(TSharedPtr<FEnvQueryResult> result)
//{
//	if (result->IsSuccsessful()) {
//		MoveToLocation(result->GetItemAsLocation(0));
//	}
//}

//AActor* AIAIAvatar_AIController::GetEmptyShelf()
//{
//	//return shelf is empty
//	UObject* object = BlackboardComp->GetValueAsObject(BlackboardKey_EmptyShelf);
//
//	return object ? Cast<AActor>(object) : nullptr;
//}

