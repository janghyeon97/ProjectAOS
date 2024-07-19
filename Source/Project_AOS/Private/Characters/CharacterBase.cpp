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

	bReplicates = true;
	bAlwaysRelevant = true;

	EnumAddFlags(CharacterState, EBaseCharacterState::Move);
	EnumAddFlags(CharacterState, EBaseCharacterState::Jump);
	EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	StatComponent->OnMovementSpeedChanged.AddDynamic(this, &ACharacterBase::ChangeMovementSpeed);

	EnumAddFlags(CharacterState, EBaseCharacterState::Move);
	EnumAddFlags(CharacterState, EBaseCharacterState::Jump);
	EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);

	LogCharacterState(CharacterState, TEXT("ACharacterBase::BeginPlay"));
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
	DOREPLIFETIME(ThisClass, ReplicatedCharacterState);
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

	ACharacterBase* DamageCauserActor = Cast<ACharacterBase>(DamageCauser);
	if (::IsValid(DamageCauserActor))
	{
		LastHitActor = DamageCauserActor;
	}

	StatComponent->SetCurrentHP(StatComponent->GetCurrentHP() - FinalDamageAmount);

	return true;
}

void ACharacterBase::ChangeMovementSpeed(float InOldMS, float InNewMS)
{
	GetCharacterMovement()->MaxWalkSpeed = InNewMS;
}


void ACharacterBase::OnRep_CharacterStateChanged()
{
	/*CharacterState = static_cast<EBaseCharacterState>(ReplicatedCharacterState);
	LogCharacterState(CharacterState, TEXT("ACharacterBase::OnRep_CharacterStateChanged"));*/
}

void ACharacterBase::ModifyCharacterState_Implementation(ECharacterStateOperation Operation, EBaseCharacterState StateFlag)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] ModifyCharacterState_Implementation called without authority."));
		return;
	}

	EBaseCharacterState NewState = CharacterState;

	LogCharacterState(CharacterState, TEXT("ACharacterBase::ModifyCharacterState Before CharacterState"));

	switch (Operation)
	{
	case ECharacterStateOperation::Add:
		EnumAddFlags(NewState, StateFlag);
		break;
	case ECharacterStateOperation::Remove:
		EnumRemoveFlags(NewState, StateFlag);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown Operation: %d"), static_cast<int32>(Operation));
		break;
	}

	CharacterState = NewState;
	SetCharacterState(static_cast<uint32>(NewState));

	UpdateCharacterState(ReplicatedCharacterState);
	LogCharacterState(CharacterState, TEXT("ACharacterBase::ModifyCharacterState After CharacterState"));
}

void ACharacterBase::UpdateCharacterState_Implementation(uint32 NewState)
{
	if (!HasAuthority())
	{
		CharacterState = static_cast<EBaseCharacterState>(NewState);
		ReplicatedCharacterState = NewState;
		LogCharacterState(CharacterState, TEXT("ACharacterBase::UpdateCharacterState"));
	}
}

// Enum 값 범위 확인을 위한 함수 추가
bool ACharacterBase::IsValidCharacterState(EBaseCharacterState State)
{
	switch (State)
	{
	case EBaseCharacterState::None:
	case EBaseCharacterState::Death:
	case EBaseCharacterState::Move:
	case EBaseCharacterState::Jump:
	case EBaseCharacterState::Ability_Q:
	case EBaseCharacterState::Ability_E:
	case EBaseCharacterState::Ability_R:
	case EBaseCharacterState::Ability_LMB:
	case EBaseCharacterState::Ability_RMB:
	case EBaseCharacterState::AbilityUsed:
	case EBaseCharacterState::SwitchAction:
		return true;
	default:
		return false;
	}
}


bool ACharacterBase::IsValidCombinedCharacterState(EBaseCharacterState State)
{
	// 모든 플래그 값이 유효한지 확인
	uint32 CombinedValue = static_cast<uint32>(State);
	UEnum* EnumPtr = StaticEnum<EBaseCharacterState>();

	for (uint32 Bit = 1; Bit <= CombinedValue; Bit <<= 1)
	{
		if ((CombinedValue & Bit) && !IsValidCharacterState(static_cast<EBaseCharacterState>(Bit)))
		{
			return false;
		}
	}
	return true;
}

void ACharacterBase::SetCharacterState(uint32 NewState)
{
	ReplicatedCharacterState = NewState;
	UpdateCharacterState(ReplicatedCharacterState);
}

void ACharacterBase::LogCharacterState(EBaseCharacterState State, const FString& Context)
{
	FString CharacterStateString;
	UEnum* EnumPtr = StaticEnum<EBaseCharacterState>();
	if (EnumPtr)
	{
		for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i) // 마지막 Enum은 MAX로 취급하고 제외
		{
			int64 Value = EnumPtr->GetValueByIndex(i);
			if (EnumHasAnyFlags(State, static_cast<EBaseCharacterState>(Value)) && Value != static_cast<int64>(EBaseCharacterState::None))
			{
				CharacterStateString += EnumPtr->GetNameStringByIndex(i) + TEXT(" ");
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[%s::%s] Actor: %s - ReplicatedCharacterState %d, CharacterState: %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"),
			*Context, *GetName(), ReplicatedCharacterState, *CharacterStateString);
	}
}
