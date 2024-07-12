// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MinionAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"


AMinionAIController::AMinionAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardComponent> BLACKBOARD (TEXT("/Game/ProjectAOS/AI/Aurora/BB_NPC_Aurora.BB_NPC_Aurora"));
	if (BLACKBOARD.Succeeded()) Blackboard = BLACKBOARD.Object;

	static ConstructorHelpers::FObjectFinder<UBehaviorTreeComponent> BRAINCOMPONENT (TEXT("/Game/ProjectAOS/AI/Aurora/BT_NPC_Aurora.BT_NPC_Aurora"));
	if (BRAINCOMPONENT.Succeeded()) BrainComponent = BRAINCOMPONENT.Object;
}

void AMinionAIController::BeginPlay()
{
	Super::BeginPlay();

	APawn* ControlledPawn = GetPawn();
	if (::IsValid(ControlledPawn))
	{
		BeginAI(ControlledPawn);
	}
}

void AMinionAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	EndAI();
}

void AMinionAIController::BeginAI(APawn* InPawn)
{
	UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(Blackboard);
	if (::IsValid(BlackboardComponent))
	{
		if (UseBlackboard(BlackboardDataAsset, BlackboardComponent))
		{
			bool bRunSucceed = RunBehaviorTree(BehaviorTree);
			ensure(bRunSucceed == true);
			//BlackboardComponent->SetValueAsVector(StartPatrolPositionKey, InPawn->GetActorLocation());
		}
	}
}

void AMinionAIController::EndAI()
{
	UBehaviorTreeComponent* BehaviroTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (::IsValid(BehaviroTreeComponent))
	{
		BehaviroTreeComponent->StopTree();
	}
}
