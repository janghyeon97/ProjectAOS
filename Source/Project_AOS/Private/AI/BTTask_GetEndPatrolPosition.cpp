// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_GetEndPatrolPosition.h"
#include "Controllers/NPCAIController.h"
#include "Characters/NonPlayerCharacterBase.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_GetEndPatrolPosition::UBTTask_GetEndPatrolPosition()
{
	NodeName = TEXT("GenEndPatrolPosition");
}

EBTNodeResult::Type UBTTask_GetEndPatrolPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	if (Result == EBTNodeResult::Failed)
	{
		return Result;
	}

	ANPCAIController* AIC = Cast<ANPCAIController>(OwnerComp.GetAIOwner());
	if (::IsValid(AIC) == false)
	{
		return Result = EBTNodeResult::Failed;
	}

	ANonPlayerCharacterBase* NPC = Cast<ANonPlayerCharacterBase>(AIC->GetPawn());
	if (::IsValid(NPC) == false)
	{
		return Result = EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NS = UNavigationSystemV1::GetNavigationSystem(NPC->GetWorld());
	if (::IsValid(NS) == false)
	{
		return Result = EBTNodeResult::Failed;
	}

	FVector StartPatrolPosition = OwnerComp.GetBlackboardComponent()->GetValueAsVector(ANPCAIController::StartPatrolPositionKey);
	FNavLocation EndPatrolLocation;

	if (NS->GetRandomPointInNavigableRadius(FVector::ZeroVector, AIC->PatrolRadius, EndPatrolLocation) == true)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(ANPCAIController::EndPatrolPositionKey, EndPatrolLocation.Location);
		return Result = EBTNodeResult::Succeeded;
	}

	return Result;
}
