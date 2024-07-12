#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Structs/DamageInfomationStruct.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	StatComponent->OnMovementSpeedChanged.AddDynamic(this, &ACharacterBase::ChangeMovementSpeed);
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamSide);
	DOREPLIFETIME(ThisClass, ObjectType);
	DOREPLIFETIME(ThisClass, CharacterState);
}

bool ACharacterBase::ValidateHit(EAbilityID AbilityID)
{
	if (AbilityID == EAbilityID::Ability_LMB)
	{
		return true;
	}
	else
	{
		bool bResult = true;
		if (OnReceiveDamageEnteredEvent.IsBound())
		{
			OnReceiveDamageEnteredEvent.Broadcast(bResult);
		}
		return bResult;
	}
}

/**
 * ACharacterBase::ReceiveDamage - 캐릭터가 받는 데미지를 처리합니다.
 * 물리, 마법, 진실 데미지 유형에 따라 최종 데미지 양을 계산하고 캐릭터의 체력을 업데이트합니다.
 *
 * @param DamageInfomation   데미지 정보 구조체로, 물리, 마법, 진실 데미지 값을 포함합니다.
 * @param EventInstigator    데미지를 일으킨 컨트롤러입니다.
 * @param DamageCauser       데미지를 발생시킨 액터입니다.
 * @return                   데미지가 성공적으로 처리되면 true를 반환합니다.
 */
bool ACharacterBase::ReceiveDamage(FDamageInfomation DamageInfomation, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamageAmount = 0;

	if (OnPreDamageCalculationEvent.IsBound())
	{
		OnPreDamageCalculationEvent.Broadcast(DamageInfomation);
	}

	if (EnumHasAnyFlags(DamageInfomation.DamageType, EDamageType::Physical) || EnumHasAnyFlags(DamageInfomation.DamageType, EDamageType::Critical))
	{
		if (EnumHasAnyFlags(DamageInfomation.DamageType, EDamageType::Critical))
		{
			float ReducedCriticalDamage = DamageInfomation.PhysicalDamage * (100 / (100 + StatComponent->GetDefensePower()));
			FinalDamageAmount += ReducedCriticalDamage;
			DamageInfomation.PhysicalDamage = ReducedCriticalDamage;
		}
		else
		{
			float ReducedPhysicalDamage = DamageInfomation.PhysicalDamage * (100 / (100 + StatComponent->GetDefensePower()));
			FinalDamageAmount += ReducedPhysicalDamage;
			DamageInfomation.PhysicalDamage = ReducedPhysicalDamage;
		}
	}

	if (EnumHasAnyFlags(DamageInfomation.DamageType, EDamageType::Magic))
	{
		float ReducedMagicDamage = DamageInfomation.MagicDamage * (100 / (100 + StatComponent->GetMagicResistance()));
		FinalDamageAmount += ReducedMagicDamage;
		DamageInfomation.MagicDamage = ReducedMagicDamage;
	}

	if (EnumHasAnyFlags(DamageInfomation.DamageType, EDamageType::TrueDamage))
	{
		FinalDamageAmount += DamageInfomation.TrueDamage;
	}
	
	if (OnPostDamageCalculationEvent.IsBound())
	{
		OnPostDamageCalculationEvent.Broadcast(FinalDamageAmount);
	}

	UE_LOG(LogTemp, Warning, TEXT("%s Received %f Damages from %s, reduced CurrentHP from %f to %f"),
		*this->GetName(), FinalDamageAmount, *DamageCauser->GetName(), StatComponent->GetCurrentHP(), StatComponent->GetCurrentHP() - FinalDamageAmount);

	StatComponent->SetCurrentHP(StatComponent->GetCurrentHP() - FinalDamageAmount);
	LastHitActor = DamageCauser;

	return true;
}



void ACharacterBase::ChangeMovementSpeed(float InOldMS, float InNewMS)
{
	GetCharacterMovement()->MaxWalkSpeed = InNewMS;
}


void ACharacterBase::OnRep_CharacterStateChanged()
{
}