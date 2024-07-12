// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "Controllers/NPCAIController.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/NonPlayerCharacterBase.h"
#include "Components/StatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Attack::UBTTask_Attack()	
{
	bNotifyTick = true;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	ANPCAIController* AIC = Cast<ANPCAIController>(OwnerComp.GetAIOwner());

	if (::IsValid(AIC))
	{
		ANonPlayerCharacterBase* NPC = Cast<ANonPlayerCharacterBase>(AIC->GetPawn());

		if (::IsValid(NPC))
		{
			AAOSCharacterBase* TargetPlayerCharacter =Cast<AAOSCharacterBase>
				(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ANPCAIController::TargetActorKey));
			if (::IsValid(TargetPlayerCharacter))
			{
				bool bResult = (NPC->GetDistanceTo(TargetPlayerCharacter) <= 200) ? true : false;
				OwnerComp.GetBlackboardComponent()->SetValueAsBool(ANPCAIController::IsPlayerDetectedKey, bResult);
			}

			if (NPC->GetIsAttacking() == false)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	ANPCAIController* AIC = Cast<ANPCAIController>(OwnerComp.GetAIOwner());
	if (::IsValid(AIC))
	{
		ANonPlayerCharacterBase* NPC = Cast<ANonPlayerCharacterBase>(AIC->GetPawn());

		if (::IsValid(NPC))
		{
			NPC->Ability_LMB();
		}
	}

	return EBTNodeResult::InProgress;
}