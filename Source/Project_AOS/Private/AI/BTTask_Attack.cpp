// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Characters/MinionBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		return;
	}

	ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
	if (!TargetCharacter)
	{
		return;
	}

	if (EnumHasAnyFlags(TargetCharacter->CharacterState, EBaseCharacterState::Death))
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
		AICharacter->ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::AttackEnded);
		AICharacter->ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Attacking);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}

	if (EnumHasAnyFlags(AICharacter->CharacterState, EBaseCharacterState::AttackEnded))
	{
		AICharacter->ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::AttackEnded);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
	if (AICharacter)
	{
		AICharacter->Ability_LMB();
	}

	return EBTNodeResult::InProgress;
}