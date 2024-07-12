// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_IsPlayerWithinRange.h"
#include "Controllers/BaseAIController.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsPlayerWithinRange::UBTDecorator_IsPlayerWithinRange()
{
	NodeName = TEXT("IsPlayerWithinRange");
}

bool UBTDecorator_IsPlayerWithinRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

    if (bResult == false)
    {
        return false;
    }

    ABaseAIController* AIC = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (::IsValid(AIC) == false)
    {
        return false;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIC->GetPawn());
    if (::IsValid(AICharacter) == false)
    {
        return false;
    }

    AAOSCharacterBase* TargetPlayerCharacter = Cast<AAOSCharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (::IsValid(TargetPlayerCharacter) == false)
    {
        return false;
    }

    bResult = (AICharacter->GetDistanceTo(TargetPlayerCharacter) <= 200) ? true : false;
    OwnerComp.GetBlackboardComponent()->SetValueAsBool(ABaseAIController::IsPlayerDetectedKey, bResult);

    return bResult;
}