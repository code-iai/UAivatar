// Copyright 2017-2021, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "IAIAvatar_AIController.generated.h"

/**
 * 
 */
UCLASS()
class UIAIAVATAR_API AIAIAvatar_AIController : public AAIController
{
	GENERATED_BODY()

public:

	AIAIAvatar_AIController(FObjectInitializer const& object_initializer = FObjectInitializer::Get());

	//AIAIAvatae_AIController(FObjectInitializer const& object_initializer);
	void BeginPlay() override;

	void OnPossess(APawn* const pawn) override;

	class UBlackboardComponent* GetBlackboard() const;

	/*UPROPERTY(EditAnywhere, Category = "AI")
		UEnvQuery* FindHidingSpotEQS;

	UFUNCTION(BlueprintCallable)
		void FindHidingSpot();*/

	//void MoveToQueryResult(TSharedPtr<FEnvQueryResult> result);

	/** Returns empty shelf. Returns null, if there is no target */
	//AActor* GetEmptyShelf();

	/** Blackboard Key Value Name */
	const FName BlackboardKey_EmptyShelf = FName("EmptyShelf");



private:

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
		class UBehaviorTreeComponent* BehaviorTreeComp;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
		class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere)
		UAIPerceptionComponent* AIPerceptionComponent;

	class UBlackboardComponent* BlackboardComp;

	class UAISenseConfig_Sight* SightConfig;

	class UAISenseConfig_Hearing* hearing_config;

	UFUNCTION()
		void OnUpdated(TArray<AActor*> const& UpdatedActors);

	void SetupPerceptionSystem();

	/*class UNavigationSystemV1* NavArea;

	FVector RandomLocation;*/
	
};
