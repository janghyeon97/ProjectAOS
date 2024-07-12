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
 * ACharacterBase::ReceiveDamage - ĳ���Ͱ� �޴� �������� ó���մϴ�.
 * ����, ����, ���� ������ ������ ���� ���� ������ ���� ����ϰ� ĳ������ ü���� ������Ʈ�մϴ�.
 *
 * @param DamageInfomation   ������ ���� ����ü��, ����, ����, ���� ������ ���� �����մϴ�.
 * @param EventInstigator    �������� ����Ų ��Ʈ�ѷ��Դϴ�.
 * @param DamageCauser       �������� �߻���Ų �����Դϴ�.
 * @return                   �������� ���������� ó���Ǹ� true�� ��ȯ�մϴ�.
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