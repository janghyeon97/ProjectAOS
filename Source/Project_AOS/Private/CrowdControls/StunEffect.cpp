// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/StunEffect.h"
#include "Characters/CharacterBase.h"
#include "Components/AbilityStatComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "GameFramework/CharacterMovementComponent.h"


void UStunEffect::ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent)
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

	APlayerController* PlayerController = Cast<APlayerController>(InTarget->GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target's controller is not a valid APlayerController."));
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(InTarget);
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
		return;
	}

	Character->PauseMontage_NetMulticast();
	Character->CancelAbility();
	Character->bReplicatedUseControllerRotationYaw = false;
	EnumAddFlags(Character->CrowdControlState, EBaseCrowdControl::Stun);

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
	}

	Target = InTarget;
	Duration = InDuration;
	Percent = InPercent;

	Character->DisableInput(PlayerController); // 입력 비활성화

	// 타이머 설정
	World->GetTimerManager().ClearTimer(TimerHandle);
	World->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			this->RemoveEffect();
		},
		Duration,
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

		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect success: Ability name %s, level %d."), *Ability->Name, Ability->CurrentLevel);

		Ability->bCanCastAbility = false; // 스킬을 사용할 수 없도록 설정
		AbilityStatComponent->BroadcastAbilityVisibility_Client(Ability->AbilityID, false);
		DisabledAbilities.Add(Ability);
	}
}


void UStunEffect::RemoveEffect()
{
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is null."));
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(Target);
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is not a valid ACharacterBase."));
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target's controller is not a valid APlayerController."));
		return;
	}

	UAbilityStatComponent* AbilityStatComponent = Character->GetAbilityStatComponent();
	if (!AbilityStatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: AbilityStatComponent is null for the target."));
		return;
	}

	// CrowdControlState 및 CharacterState 업데이트
	EnumRemoveFlags(Character->CrowdControlState, EBaseCrowdControl::Stun);
	EnumAddFlags(Character->CharacterState, EBaseCharacterState::Move);
	EnumAddFlags(Character->CharacterState, EBaseCharacterState::SwitchAction);
	Character->EnableInput(PlayerController);
	Character->StopAllMontages_Server(0.25f, true);
	Character->bReplicatedUseControllerRotationYaw = true;

	// AbilityStatComponent 및 DisabledAbilities 처리
	if (DisabledAbilities.Num() > 0)
	{
		for (FAbilityDetails* Ability : DisabledAbilities)
		{
			Ability->bCanCastAbility = true;	
			if (Ability->bAbilityReady)
			{
				AbilityStatComponent->BroadcastAbilityVisibility_Client(Ability->AbilityID, true);
			}
		}
		DisabledAbilities.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect: DisabledAbilities array is empty."));
	}

	ReturnEffect();
}


void UStunEffect::ReturnEffect()
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
	if (Character->ActiveEffects.Remove(EBaseCrowdControl::Stun) > 0)
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

void UStunEffect::Reset()
{
	DisabledAbilities.Empty();
	Duration = 0.0f;
	Percent = 0.0f;
}

float UStunEffect::GetDuration() const
{
	return Duration;
}

float UStunEffect::GetPercent() const
{
	return Percent;
}
