// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_IsAbilityReady.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Components/AbilityStatComponent.h"

UBTDecorator_IsAbilityReady::UBTDecorator_IsAbilityReady()
{
	NodeName = TEXT("Is Ability Ready");
}

bool UBTDecorator_IsAbilityReady::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

    if (!bResult)
    {
        UE_LOG(LogTemp, Warning, TEXT("Super::CalculateRawConditionValue returned false"));
        return false;
    }

    AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get AIController"));
        return false;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get AICharacter from AIController's Pawn"));
        return false;
    }

    UAbilityStatComponent* AbilityStatComponent = AICharacter->GetAbilityStatComponent();
    if (!AbilityStatComponent)
    {
        return false;
    }

    UE_LOG(LogTemp, Error, TEXT("[UBTDecorator_IsAbilityReady] %s Ability cooldown remaing %f."), *AICharacter->GetName(), AbilityStatComponent->Ability_LMB_Info.Cooldown);
    return AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_LMB);
}
