// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/NPCAIController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"

const float ABaseAIController::PatrolRepeatInterval(3.f);
const float ABaseAIController::PatrolRadius(500.f);
const FName ABaseAIController::StartPatrolPositionKey(TEXT("StartPatrolPosition"));
const FName ABaseAIController::EndPatrolPositionKey(TEXT("EndPatrolPosition"));
const FName ABaseAIController::TargetActorKey(TEXT("TargetActor"));
const FName ABaseAIController::IsPlayerDetectedKey(TEXT("IsPlayerDetected"));
const FName ABaseAIController::IsGetCrowdControl(TEXT("IsGetCrowdControl"));

ANPCAIController::ANPCAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardComponent> BLACKBOARD(TEXT("/Game/ProjectAOS/AI/Aurora/BB_NPC_Aurora.BB_NPC_Aurora"));
	if (BLACKBOARD.Succeeded()) Blackboard = BLACKBOARD.Object;

	static ConstructorHelpers::FObjectFinder<UBehaviorTreeComponent> BRAINCOMPONENT(TEXT("/Game/ProjectAOS/AI/Aurora/BT_NPC_Aurora.BT_NPC_Aurora"));
	if (BRAINCOMPONENT.Succeeded()) BrainComponent = BRAINCOMPONENT.Object;
}

void ANPCAIController::BeginAI(APawn* InPawn)
{
	Super::BeginAI(InPawn);

	UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(Blackboard);
	if (::IsValid(BlackboardComponent))
	{
		if (UseBlackboard(BlackboardDataAsset, BlackboardComponent))
		{
			bool bRunSucceed = RunBehaviorTree(BehaviorTree);
			ensure(bRunSucceed == true);
			BlackboardComponent->SetValueAsVector(StartPatrolPositionKey, InPawn->GetActorLocation());
		}
	}
}

void ANPCAIController::EndAI()
{
	Super::EndAI();

	UBehaviorTreeComponent* BehaviroTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (::IsValid(BehaviroTreeComponent))
	{
		BehaviroTreeComponent->StopTree();
	}
}

void ANPCAIController::OnPatrolTimerElapsed()
{
	APawn* ControlledPawn = GetPawn();
	if (::IsValid(ControlledPawn))
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
		if (::IsValid(NavSystem))
		{
			FNavLocation NextLocation;
			if (true == NavSystem->GetRandomPointInNavigableRadius(FVector::ZeroVector, PatrolRadius, NextLocation))
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NextLocation.Location);
			}
		}
	}
}
