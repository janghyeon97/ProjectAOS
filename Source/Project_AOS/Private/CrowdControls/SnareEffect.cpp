// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/SnareEffect.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "GameFramework/CharacterMovementComponent.h"


void USnareEffect::ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent)
{
    if (!InTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is null."));
        return;
    }

    UWorld* World = InTarget->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyEffect failed: Target's GetWorld() returned null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(InTarget);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
    if (MovementComponent)
    {
        MovementComponent->StopMovementImmediately();
        EnumAddFlags(Character->CrowdControlState, EBaseCrowdControl::Snare);
    }

    Target = InTarget;
    Duration = InDuration;
    Percent = InDuration;

    World->GetTimerManager().ClearTimer(TimerHandle);
    World->GetTimerManager().SetTimer(
        TimerHandle,
        [this]()
        {
            this->RemoveEffect();
        },
        InDuration,
        false
    );

    UAbilityStatComponent* AbilityStatComponent = Character->GetAbilityStatComponent();
    if (!AbilityStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: AbilityStatComponent is null for the target."));
        return;
    }

    TArray<FAbilityDetails*> Abilities = AbilityStatComponent->GetAbilityDetailsPtrs();
    if (Abilities.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Abilities array is empty."));
        return;
    }

    for (FAbilityDetails* Ability : Abilities)  // Abilities는 모든 스킬 정보를 담은 배열
    {
        if (Ability->Name.IsEmpty() && Ability->AbilityDetection == EAbilityDetection::None)
        {
            continue;
        }

        if (EnumHasAnyFlags(Ability->AbilityDetection, EAbilityDetection::Dash) || EnumHasAnyFlags(Ability->AbilityDetection, EAbilityDetection::Blink))
        {
            Ability->bCanCastAbility = false; // 스킬을 사용할 수 없도록 설정
            AbilityStatComponent->BroadcastAbilityVisibility_Client(Ability->AbilityID, false);
            DisabledAbilities.Add(Ability);
            UE_LOG(LogTemp, Log, TEXT("%s skill has been disabled due to Snare effect."), *Ability->Name);
        }
    }
}

void USnareEffect::RemoveEffect()
{
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(Target);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    EnumRemoveFlags(Character->CrowdControlState, EBaseCrowdControl::Snare);
    EnumAddFlags(Character->CharacterState, EBaseCharacterState::Move);
    EnumAddFlags(Character->CharacterState, EBaseCharacterState::SwitchAction);
    
    // Abilities 배열의 크기와 유효성 검사
    if (DisabledAbilities.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Abilities array is empty."));
        return;
    }

    for (FAbilityDetails* Ability : DisabledAbilities)
    {
        if (Ability && !Ability->Name.IsEmpty() && Ability->AbilityDetection != EAbilityDetection::None)
        {
            Ability->bCanCastAbility = true;
            UE_LOG(LogTemp, Log, TEXT("%s skill has been re-enabled after Snare effect."), *Ability->Name);
        }
    }

    DisabledAbilities.Empty();
    ReturnEffect();
}

void USnareEffect::ReturnEffect()
{
    if (!::IsValid(Target))
    {
        UE_LOG(LogTemp, Warning, TEXT("ReturnEffect failed: Target is null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(Target);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    // ActiveEffects에서 효과 제거
    if (Character->ActiveEffects.Remove(EBaseCrowdControl::Snare) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Removed Stun effect from ActiveEffects."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect: ActiveEffects does not contain Stun effect."));
    }

    // 효과 객체를 풀로 반환
    if (UCrowdControlManager* CrowdControlManager = UCrowdControlManager::Get())
    {
        if (::IsValid(CrowdControlManager))
        {
            CrowdControlManager->ReturnEffect(EBaseCrowdControl::Stun, this);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Retrieved CrowdControlManager instance is invalid."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Could not retrieve UCrowdControlManager instance."));
    }
}

void USnareEffect::Reset()
{
    DisabledAbilities.Empty();
    Duration = 0.0f;
    Percent = 0.0f;
}



float USnareEffect::GetDuration() const
{
    return 0.0f;
}

float USnareEffect::GetPercent() const
{
    return 0.0f;
}
