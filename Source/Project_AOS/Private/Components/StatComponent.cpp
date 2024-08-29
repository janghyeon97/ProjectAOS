#include "Components/StatComponent.h"
#include "DataProviders/CharacterDataProviderBase.h"
#include "Characters/CharacterBase.h"
#include "Game/AOSGameInstance.h"
#include "Game/AOSPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


typedef TFunction<void(float)> ModifyFunction;


UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;

	CurrentLevel = 1;
	MaxLevel = 18;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

}

void UStatComponent::InitStatComponent(ICharacterDataProviderInterface* InDataProvider, const FName& InRowName)
{
	if (!InDataProvider)
	{
		UE_LOG(LogTemp, Error, TEXT("UStatComponent::InitStatComponent - DataProvider is null"));
		return;
	}

	DataProvider = InDataProvider;
	RowName = InRowName;

	// 데이터 제공자를 통해 행을 가져와서 스탯을 초기화합니다.
	const FStatTableRow* StatRow = DataProvider->GetCharacterStat(InRowName, 1); // 레벨 1 기준으로 기본 스탯 초기화
	if (StatRow)
	{
		// 기본 스탯 설정
		BaseMaxHP = StatRow->MaxHP;
		BaseMaxMP = StatRow->MaxMP;
		BaseHealthRegeneration = StatRow->HealthRegeneration;
		BaseManaRegeneration = StatRow->ManaRegeneration;
		BaseAttackDamage = StatRow->AttackDamage;
		BaseDefensePower = StatRow->DefensePower;
		BaseMagicResistance = StatRow->MagicResistance;
		BaseAttackSpeed = StatRow->AttackSpeed;
		BaseCriticalChance = StatRow->CriticalChance;
		BaseMovementSpeed = StatRow->MovementSpeed;

		// 현재 스탯 초기화 (기본 스탯과 동일하게 설정)
		CurrentHP = BaseMaxHP;
		CurrentMP = BaseMaxMP;

		// EXP와 Level은 기본 스탯 없이 초기화
		MaxEXP = StatRow->MaxEXP;
		CurrentEXP = 0; 
		MaxLevel = 18;  
		CurrentLevel = 1;

		// 누적된 변경값은 초기화 시점에서는 모두 0이어야 합니다.
		AccumulatedFlatMaxHP = 0;
		AccumulatedFlatMaxMP = 0;
		AccumulatedFlatMaxEXP = 0;
		AccumulatedFlatHealthRegeneration = 0;
		AccumulatedFlatManaRegeneration = 0;
		AccumulatedFlatAttackDamage = 0;
		AccumulatedFlatDefensePower = 0;
		AccumulatedFlatMagicResistance = 0;
		AccumulatedFlatCriticalChance = 0;
		AccumulatedFlatAttackSpeed = 0;
		AccumulatedFlatMovementSpeed = 0;

		AccumulatedPercentAttackSpeed = 0;
		AccumulatedPercentMovementSpeed = 0;

		// 모든 스탯을 초기화한 후, 현재 스탯을 재계산합니다.
		RecalculateStats();

		UE_LOG(LogTemp, Log, TEXT("UStatComponent::InitStatComponent - Stats initialized for RowName: %s"), *RowName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UStatComponent::InitStatComponent - Could not find row for RowName: %s"), *RowName.ToString());
	}
}

void UStatComponent::RecalculateStats()
{
	// 누적된 플랫 변경값을 기본 스탯에 더하여 현재 스탯을 계산
	SetMaxHP(BaseMaxHP + AccumulatedFlatMaxHP);
	SetMaxMP(BaseMaxMP + AccumulatedFlatMaxMP);
	SetHealthRegeneration(BaseHealthRegeneration + AccumulatedFlatHealthRegeneration);
	SetManaRegeneration(BaseManaRegeneration + AccumulatedFlatManaRegeneration);
	SetAttackDamage(BaseAttackDamage + AccumulatedFlatAttackDamage);
	SetAbilityPower(BaseAbilityPower + AccumulatedFlatAbilityPower);
	SetDefensePower(BaseDefensePower + AccumulatedFlatDefensePower);
	SetMagicResistance(BaseMagicResistance + AccumulatedFlatMagicResistance);
	SetAbilityHaste(BaseAbilityHaste + AccumulatedFlatAbilityHaste);
	SetCriticalChance(BaseCriticalChance + AccumulatedFlatCriticalChance);

	// 퍼센트로 증가하는 스탯 계산
	float NewAttackSpeed = BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed;
	SetAttackSpeed(NewAttackSpeed);

	float NewMovementSpeed = BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed;
	SetMovementSpeed(NewMovementSpeed);

	// 현재 HP와 MP가 최대값을 초과하지 않도록 보정
	SetCurrentHP(MaxHP);
	SetCurrentMP(MaxMP);
}



void UStatComponent::OnHUDInitializationCompleted_Server_Implementation()
{
	//UE_LOG(LogTemp, Warning, TEXT("UStatComponent::OnHUDInitializationCompleted_Server - %d"), CurrentLevel);
	//OnCurrentLevelChanged.Broadcast(0, CurrentLevel);
}

void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MaxHP);
	DOREPLIFETIME(ThisClass, CurrentHP);
	DOREPLIFETIME(ThisClass, MaxMP);
	DOREPLIFETIME(ThisClass, CurrentMP);
	DOREPLIFETIME(ThisClass, MaxEXP);
	DOREPLIFETIME(ThisClass, CurrentEXP);
	DOREPLIFETIME(ThisClass, MaxLevel);
	DOREPLIFETIME(ThisClass, CurrentLevel);
	DOREPLIFETIME(ThisClass, HealthRegeneration);
	DOREPLIFETIME(ThisClass, ManaRegeneration);
	DOREPLIFETIME(ThisClass, AttackDamage);
	DOREPLIFETIME(ThisClass, AbilityPower);
	DOREPLIFETIME(ThisClass, DefensePower);
	DOREPLIFETIME(ThisClass, MagicResistance);
	DOREPLIFETIME(ThisClass, AttackSpeed);
	DOREPLIFETIME(ThisClass, AbilityHaste);
	DOREPLIFETIME(ThisClass, CriticalChance);
	DOREPLIFETIME(ThisClass, MovementSpeed);

	// Base stats
	DOREPLIFETIME(ThisClass, BaseMaxHP);
	DOREPLIFETIME(ThisClass, BaseMaxMP);
	DOREPLIFETIME(ThisClass, BaseCurrentMP);
	DOREPLIFETIME(ThisClass, BaseHealthRegeneration);
	DOREPLIFETIME(ThisClass, BaseManaRegeneration);
	DOREPLIFETIME(ThisClass, BaseAttackDamage);
	DOREPLIFETIME(ThisClass, BaseAbilityPower);
	DOREPLIFETIME(ThisClass, BaseDefensePower);
	DOREPLIFETIME(ThisClass, BaseMagicResistance);
	DOREPLIFETIME(ThisClass, BaseAttackSpeed);
	DOREPLIFETIME(ThisClass, BaseMovementSpeed);
	DOREPLIFETIME(ThisClass, BaseAbilityHaste);
	DOREPLIFETIME(ThisClass, BaseCriticalChance);
}

#pragma region Setter

void UStatComponent::SetMaxHP(float InMaxHP)
{
	float NewMaxHP = FMath::Clamp(InMaxHP, 0.0f, 99999.f);
	UE_LOG(LogTemp, Log, TEXT("%s Set MaxHealth :: %f -> %f"), *GetOwner()->GetName(), MaxHP, NewMaxHP);

	OnMaxHPChanged_NetMulticast(MaxHP, NewMaxHP);
	MaxHP = NewMaxHP;
}

void UStatComponent::SetCurrentHP(float InCurrentHP)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change CurrentHealth :: %f -> %f"), *GetOwner()->GetName(), CurrentHP, InCurrentHP);

	float NewCurrentHP = FMath::Clamp<float>(InCurrentHP, 0, MaxHP);
	OnCurrentHPChanged_NetMulticast(CurrentHP, NewCurrentHP);
	CurrentHP = NewCurrentHP;

	if (CurrentHP < KINDA_SMALL_NUMBER)
	{
		OnOutOfCurrentHP_NetMulticast();
		CurrentHP = 0.f;
		return;
	}

	if (CurrentHP != MaxHP)
	{
		if (OnHPNotEqualsMaxHP.IsBound()) OnHPNotEqualsMaxHP.Broadcast();
	}
	else
	{
		if (OnHPEqualsMaxHP.IsBound()) OnHPEqualsMaxHP.Broadcast();
	}
}

void UStatComponent::SetMaxMP(float InMaxMP)
{
	float NewMaxMP = FMath::Clamp(InMaxMP, 0.0f, 99999.f); // MaxMaxMP는 필요 시 정의
	UE_LOG(LogTemp, Log, TEXT("%s Set MaxMP :: %f -> %f"), *GetOwner()->GetName(), MaxMP, NewMaxMP);

	OnMaxMPChanged_NetMulticast(MaxMP, NewMaxMP);
	MaxMP = NewMaxMP;
}

void UStatComponent::SetCurrentMP(float InCurrentMP)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change CurrentResource :: %f -> %f"), *GetOwner()->GetName(), CurrentMP, InCurrentMP);

	float NewCurrentMP = FMath::Clamp<float>(InCurrentMP, 0, MaxMP);
	OnCurrentMPChanged_NetMulticast(CurrentMP, NewCurrentMP);
	CurrentMP = NewCurrentMP;

	if (CurrentMP != MaxMP)
	{
		if (OnMPNotEqualsMaxMP.IsBound()) OnMPNotEqualsMaxMP.Broadcast();
	}
	else
	{
		if (OnMPEqualsMaxMP.IsBound()) OnMPEqualsMaxMP.Broadcast();
	}
}

void UStatComponent::SetMaxEXP(float InMaxEXP)
{
	UE_LOG(LogTemp, Log, TEXT("Change MaxEXP :: %f -> %f"), MaxEXP, InMaxEXP);

	float NewMaxEXP = FMath::Clamp<int32>(InMaxEXP, 0, 99999);
	OnMaxEXPChanged_NetMulticast(MaxEXP, NewMaxEXP);
	MaxEXP = NewMaxEXP;
}

void UStatComponent::SetCurrentEXP(float InCurrentEXP)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change CurrentEXP :: %f -> %f"), *GetOwner()->GetName(), CurrentEXP, InCurrentEXP);

	if (CurrentLevel < MaxLevel)
	{
		if (InCurrentEXP >= MaxEXP)
		{
			// 레벨업 로직
			float OverflowEXP = InCurrentEXP - MaxEXP;
			SetCurrentLevel(FMath::Clamp<int32>(GetCurrentLevel() + 1, 1, MaxLevel));
			SetCurrentEXP(OverflowEXP); // 남은 경험치로 설정
		}
		else
		{
			CurrentEXP = FMath::Clamp<float>(InCurrentEXP, 0.f, MaxEXP);
			OnCurrentEXPChanged_NetMulticast(CurrentEXP, InCurrentEXP);
		}
	}
	else
	{
		// 최대 레벨 도달 시, 경험치를 추가로 계산하지 않음
		CurrentEXP = MaxEXP;
		OnCurrentEXPChanged_NetMulticast(CurrentEXP, InCurrentEXP);
	}
}

void UStatComponent::SetCurrentLevel(int32 InCurrentLevel)
{
	if (!DataProvider)
	{
		UE_LOG(LogTemp, Error, TEXT("SetCurrentLevel - DataProvider is not valid."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Set CurrentLevel :: %d -> %d"), CurrentLevel, InCurrentLevel);

	int32 NewCurrentLevel = FMath::Clamp<int32>(InCurrentLevel, 1, MaxLevel);
	OnCurrentLevelChanged_NetMulticast(CurrentLevel, NewCurrentLevel);

	// 레벨업에 따른 스탯 업데이트
	const FStatTableRow* NewLevelStatRow = DataProvider->GetCharacterStat(RowName, NewCurrentLevel);

	if (NewLevelStatRow)
	{
		const float OldMaxHP = BaseMaxHP;
		const float OldMaxMP = BaseMaxMP;
		

		// 기초 스탯을 새로운 레벨의 값으로 업데이트
		BaseMaxHP = NewLevelStatRow->MaxHP;
		BaseMaxMP = NewLevelStatRow->MaxMP;
		BaseHealthRegeneration = NewLevelStatRow->HealthRegeneration;
		BaseManaRegeneration = NewLevelStatRow->ManaRegeneration;
		BaseAttackDamage = NewLevelStatRow->AttackDamage;
		BaseDefensePower = NewLevelStatRow->DefensePower;
		BaseMagicResistance = NewLevelStatRow->MagicResistance;
		BaseAttackSpeed = NewLevelStatRow->AttackSpeed;
		BaseMovementSpeed = NewLevelStatRow->MovementSpeed;

		// 업데이트된 기초 스탯을 적용하여 현재 스탯 계산
		SetMaxHP(BaseMaxHP + AccumulatedFlatMaxHP);
		SetMaxMP(BaseMaxMP + AccumulatedFlatMaxMP);
		SetHealthRegeneration(BaseHealthRegeneration + AccumulatedFlatHealthRegeneration);
		SetManaRegeneration(BaseManaRegeneration + AccumulatedFlatManaRegeneration);
		SetAttackDamage(BaseAttackDamage + AccumulatedFlatAttackDamage);
		SetDefensePower(BaseDefensePower + AccumulatedFlatDefensePower);
		SetMagicResistance(BaseMagicResistance + AccumulatedFlatMagicResistance);
		SetAbilityHaste(BaseAbilityHaste + AccumulatedFlatAbilityHaste);
		SetCriticalChance(BaseCriticalChance + AccumulatedFlatCriticalChance);

		// 퍼센트로 증가하는 스탯 계산 (공격 속도와 이동 속도는 퍼센트 증가)
		SetAttackSpeed(BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed);
		SetMovementSpeed(BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed);

		// 현재 HP와 MP를 새 최대값에 맞게 업데이트
		SetCurrentHP(CurrentHP + (BaseMaxHP- OldMaxHP));
		SetCurrentMP(CurrentMP + (BaseMaxMP - OldMaxMP));

		// 경험치와 레벨 정보 업데이트
		SetMaxEXP(NewLevelStatRow->MaxEXP);
		SetCurrentEXP(CurrentEXP);

		UE_LOG(LogTemp, Log, TEXT("UStatComponent::SetCurrentLevel - Stats updated for RowName: %s"), *RowName.ToString());
	}

	CurrentLevel = NewCurrentLevel;
}

void UStatComponent::SetHealthRegeneration(float InHealthRegeneration)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change HealthRegeneration :: %f -> %f"), *GetOwner()->GetName(), HealthRegeneration, InHealthRegeneration);

	float NewHealthRegeneration = FMath::Clamp<float>(InHealthRegeneration, 0, 9999.f);
	OnHealthRegenerationChanged_NetMulticast(HealthRegeneration, NewHealthRegeneration);
	HealthRegeneration = NewHealthRegeneration;
}

void UStatComponent::SetManaRegeneration(float InManaRegeneration)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change ManaRegeneration :: %f -> %f"), *GetOwner()->GetName(), ManaRegeneration, InManaRegeneration);

	float NewManaRegeneration = FMath::Clamp<float>(InManaRegeneration, 0, 9999.f);
	OnManaRegenerationChanged_NetMulticast(ManaRegeneration, NewManaRegeneration);
	ManaRegeneration = NewManaRegeneration;
}

void UStatComponent::SetAttackDamage(float InAttackDamage)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change AttackDamage :: %f -> %f"), *GetOwner()->GetName(), AttackDamage, InAttackDamage);

	float NewAttackDamage = FMath::Clamp<float>(InAttackDamage, 0, 99999.f);
	OnAttackDamageChanged_NetMulticast(AttackDamage, NewAttackDamage);
	AttackDamage = NewAttackDamage;
}

void UStatComponent::SetAbilityPower(float InAbilityPower)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change AbilityPower :: %f -> %f"), *GetOwner()->GetName(), AbilityPower, InAbilityPower);

	float NewAbilityPower = FMath::Clamp<float>(InAbilityPower, 0, 99999.f);
	OnAbilityPowerChanged_NetMulticast(AbilityPower, NewAbilityPower);
	AbilityPower = NewAbilityPower;
}

void UStatComponent::SetDefensePower(float InDefensePower)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change DefensePower :: %f -> %f"), *GetOwner()->GetName(), DefensePower, InDefensePower);

	float NewDefensePower = FMath::Clamp<float>(InDefensePower, 0, 9999.f);
	OnDefensePowerChanged_NetMulticast(DefensePower, NewDefensePower);
	DefensePower = NewDefensePower;
}

void UStatComponent::SetMagicResistance(float InMagicResistance)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change MagicResistance :: %f -> %f"), *GetOwner()->GetName(), MagicResistance, InMagicResistance);

	float NewMagicResistance = FMath::Clamp<float>(InMagicResistance, 0, 9999.f);
	OnMagicResistanceChanged_NetMulticast(MagicResistance, NewMagicResistance);
	MagicResistance = NewMagicResistance;
}

void UStatComponent::SetAbilityHaste(int32 InAbilityHaste)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change AbilityHaste :: %d -> %d"), *GetOwner()->GetName(), AbilityHaste, InAbilityHaste);

	int32 NewAbilityHaste = FMath::Clamp<int32>(InAbilityHaste, 0, 300.f);
	OnAbilityHasteChanged_NetMulticast(AbilityHaste, NewAbilityHaste);
	AbilityHaste = NewAbilityHaste;
}

void UStatComponent::SetAttackSpeed(float InAttackSpeed)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change AttackSpeed :: %f -> %f"), *GetOwner()->GetName(), AttackSpeed, InAttackSpeed);

	float NewAttackSpeed = FMath::Clamp<float>(InAttackSpeed, 0, 2.5f);
	OnAttackSpeedChanged_NetMulticast(AttackSpeed, NewAttackSpeed);
	AttackSpeed = NewAttackSpeed;
}

void UStatComponent::SetCriticalChance(int32 InCriticalChance)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change CriticalChance :: %d -> %d"), *GetOwner()->GetName(), CriticalChance, InCriticalChance);

	int32 NewCriticalChance = FMath::Clamp<int32>(InCriticalChance, 0, 100);
	OnCriticalChanceChanged_NetMulticast(CriticalChance, NewCriticalChance);
	CriticalChance = NewCriticalChance;
}

void UStatComponent::SetMovementSpeed(float InMovementSpeed)
{
	UE_LOG(LogTemp, Log, TEXT("%s Change MovementSpeed :: %f -> %f"), *GetOwner()->GetName(), MovementSpeed, InMovementSpeed);

	float NewMovementSpeed = FMath::Clamp<float>(InMovementSpeed, 0, 9999.f);
	OnMovementSpeedChanged_NetMulticast(MovementSpeed, NewMovementSpeed);
	MovementSpeed = NewMovementSpeed;
}


#pragma endregion

#pragma region Modifiers

void UStatComponent::ModifyCurrentHP(float Delta)
{
	SetCurrentHP(CurrentHP + Delta);
}

void UStatComponent::ModifyCurrentMP(float Delta)
{
	SetCurrentMP(CurrentMP + Delta);
}

void UStatComponent::ModifyCurrentEXP(float Delta)
{
	SetCurrentEXP(CurrentEXP + Delta);
}

void UStatComponent::ModifyAccumulatedPercentAttackSpeed(float Delta)
{
	AccumulatedPercentAttackSpeed += Delta;
	float NewAttackSpeed = BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed;
	SetAttackSpeed(NewAttackSpeed);
}

void UStatComponent::ModifyAccumulatedFlatAttackSpeed(float Delta)
{
	AccumulatedFlatAttackSpeed += Delta;
	float NewAttackSpeed = BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed;
	SetAttackSpeed(NewAttackSpeed);
}

/**
 * @brief 이동 속도에 대한 퍼센트 변화를 누적하여 이동 속도를 조정합니다.
 *
 * 이 함수는 주어진 Delta 값을 누적된 퍼센트 변화에 추가하고, 이를 기반으로
 * 캐릭터의 최종 이동 속도를 계산합니다. 퍼센트 변화는 기본 이동 속도에 곱해지며,
 * 추가적인 고정 이동 속도(AccumulatedFlatMovementSpeed)도 고려됩니다.
 *
 * AccumulatedPercentMovementSpeed:
 * - 이 변수는 캐릭터의 이동 속도에 누적된 퍼센트 변화를 저장합니다.
 * - 예를 들어, -10%의 둔화 효과와 +20%의 속도 증가 효과가 적용되면, 10%로 최종 저장됩니다.
 * - 기본 이동 속도(BaseMovementSpeed)에 곱하여 최종 이동 속도를 계산하는 데 사용됩니다.
 *
 * @param Delta 이동 속도에 적용할 퍼센트 변화량 (음수일 경우 감소, 양수일 경우 증가)
 */
void UStatComponent::ModifyAccumulatedPercentMovementSpeed(float Delta)
{
	AccumulatedPercentMovementSpeed += Delta;
	float NewMovementSpeed = BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed;
	SetMovementSpeed(NewMovementSpeed);
}

/**
 * @brief 이동 속도에 대한 고정 값 변화를 누적하여 이동 속도를 조정합니다.
 *
 * 이 함수는 주어진 Delta 값을 누적된 고정 이동 속도에 추가하고, 이를 기반으로
 * 캐릭터의 최종 이동 속도를 계산합니다. 고정 이동 속도는 기본 이동 속도에 더해지며,
 * 퍼센트 변화(AccumulatedPercentMovementSpeed)도 고려됩니다.
 *
 * AccumulatedPercentMovementSpeed:
 * - 이 변수는 캐릭터의 이동 속도에 누적된 퍼센트 변화를 저장합니다.
 * - 이 함수에서는 직접적으로 수정되지 않지만, 최종 이동 속도 계산에 사용됩니다.
 *
 * @param Delta 이동 속도에 적용할 고정 값 변화량 (음수일 경우 감소, 양수일 경우 증가)
 */
void UStatComponent::ModifyAccumulatedFlatMovementSpeed(float Delta)
{
	AccumulatedFlatMovementSpeed += Delta;
	float NewMovementSpeed = BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed;
	SetMovementSpeed(NewMovementSpeed);
}



void UStatComponent::ModifyAccumulatedFlatMaxHP(float Delta)
{
	AccumulatedFlatMaxHP += Delta;
	float NewMaxHP = BaseMaxHP + AccumulatedFlatMaxHP;
	SetMaxHP(NewMaxHP);
}

void UStatComponent::ModifyAccumulatedFlatMaxMP(float Delta)
{
	AccumulatedFlatMaxMP += Delta;
	float NewMaxMP = BaseMaxMP + AccumulatedFlatMaxMP;
	SetMaxMP(NewMaxMP);
}

void UStatComponent::ModifyAccumulatedFlatHealthRegeneration(float Delta)
{
	AccumulatedFlatHealthRegeneration += Delta;
	float NewHealthRegeneration = BaseHealthRegeneration + AccumulatedFlatHealthRegeneration;
	SetHealthRegeneration(NewHealthRegeneration);
}

void UStatComponent::ModifyAccumulatedFlatManaRegeneration(float Delta)
{
	AccumulatedFlatManaRegeneration += Delta;
	float NewManaRegeneration = BaseManaRegeneration + AccumulatedFlatManaRegeneration;
	SetManaRegeneration(NewManaRegeneration);
}

void UStatComponent::ModifyAccumulatedFlatAttackDamage(float Delta)
{
	AccumulatedFlatAttackDamage += Delta;
	float NewAttackDamage = BaseAttackDamage + AccumulatedFlatAttackDamage;
	SetAttackDamage(NewAttackDamage);
}

void UStatComponent::ModifyAccumulatedFlatAbilityPower(float Delta)
{
	AccumulatedFlatAbilityPower += Delta;
	float NewAbilityPower = BaseAbilityPower + AccumulatedFlatAbilityPower;
	SetAbilityPower(NewAbilityPower);
}

void UStatComponent::ModifyAccumulatedFlatDefensePower(float Delta)
{
	AccumulatedFlatDefensePower += Delta;
	float NewDefensePower = BaseDefensePower + AccumulatedFlatDefensePower;
	SetDefensePower(NewDefensePower);
}

void UStatComponent::ModifyAccumulatedFlatMagicResistance(float Delta)
{
	AccumulatedFlatMagicResistance += Delta;
	float NewMagicResistance = BaseMagicResistance + AccumulatedFlatMagicResistance;
	SetMagicResistance(NewMagicResistance);
}

void UStatComponent::ModifyAccumulatedFlatAbilityHaste(int32 Delta)
{
	AccumulatedFlatAbilityHaste += Delta;
	int32 NewAbilityHaste = BaseAbilityHaste + AccumulatedFlatAbilityHaste;
	SetAbilityHaste(NewAbilityHaste);
}

void UStatComponent::ModifyAccumulatedFlatCriticalChance(int32 Delta)
{
	AccumulatedFlatCriticalChance += Delta;
	int32 NewCriticalChance = BaseCriticalChance + AccumulatedFlatCriticalChance;
	SetCriticalChance(NewCriticalChance);
}

#pragma endregion


typedef TFunction<void(float)> ModifyFunction;

int32 UStatComponent::ApplyFlatBuff(ECharacterStatType StatType, float FlatChange, float Duration)
{
	static const TMap<ECharacterStatType, ModifyFunction> ModifyFlatFunctions = {
		{ ECharacterStatType::MaxHealth,          [this](float Delta) { ModifyAccumulatedFlatMaxHP(Delta); } },
		{ ECharacterStatType::CurrentHealth,      [this](float Delta) { ModifyCurrentHP(Delta); } },
		{ ECharacterStatType::MaxMana,            [this](float Delta) { ModifyAccumulatedFlatMaxMP(Delta); } },
		{ ECharacterStatType::CurrentMana,        [this](float Delta) { ModifyCurrentMP(Delta); } },
		{ ECharacterStatType::HealthRegeneration, [this](float Delta) { ModifyAccumulatedFlatHealthRegeneration(Delta); } },
		{ ECharacterStatType::ManaRegeneration,   [this](float Delta) { ModifyAccumulatedFlatManaRegeneration(Delta); } },
		{ ECharacterStatType::AttackDamage,       [this](float Delta) { ModifyAccumulatedFlatAttackDamage(Delta); } },
		{ ECharacterStatType::AbilityPower,       [this](float Delta) { ModifyAccumulatedFlatAbilityPower(Delta); } },
		{ ECharacterStatType::DefensePower,       [this](float Delta) { ModifyAccumulatedFlatDefensePower(Delta); } },
		{ ECharacterStatType::MagicResistance,    [this](float Delta) { ModifyAccumulatedFlatMagicResistance(Delta); } },
		{ ECharacterStatType::AttackSpeed,        [this](float Delta) { ModifyAccumulatedFlatAttackSpeed(Delta); } },
		{ ECharacterStatType::AbilityHaste,       [this](float Delta) { ModifyAccumulatedFlatAbilityHaste(Delta); } },
		{ ECharacterStatType::CriticalChance,     [this](float Delta) { ModifyAccumulatedFlatCriticalChance(Delta); } },
		{ ECharacterStatType::MovementSpeed,      [this](float Delta) { ModifyAccumulatedFlatMovementSpeed(Delta); } }
	};

	if (ModifyFlatFunctions.Contains(StatType))
	{
		ModifyFlatFunctions[StatType](FlatChange);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UStatComponent::ApplyFlatBuff] Unsupported StatType for ApplyFlatBuff"));
		return -1;
	}

	// 버프의 인덱스 값을 생성합니다.
	int32 BuffIndex = ActiveBuffs.Num();
	FActiveBuff NewBuff(BuffIndex, StatType, FlatChange, FTimerHandle());

	GetWorld()->GetTimerManager().SetTimer(NewBuff.TimerHandle, [this, StatType, FlatChange, BuffIndex]()
		{
			if (ModifyFlatFunctions.Contains(StatType))
			{
				ModifyFlatFunctions[StatType](-FlatChange);
			}

			ActiveBuffs.RemoveAll([BuffIndex](const FActiveBuff& Buff) { return Buff.Index == BuffIndex; });
		}, Duration, false);

	ActiveBuffs.Add(NewBuff);

	return BuffIndex;
}

int32 UStatComponent::ApplyPercentBuff(ECharacterStatType StatType, float PercentChange, float Duration)
{
	static const TMap<ECharacterStatType, ModifyFunction> ModifyPercentFunctions = {
		{ ECharacterStatType::AttackSpeed,        [this](float Delta) { ModifyAccumulatedPercentAttackSpeed(Delta); } },
		{ ECharacterStatType::MovementSpeed,      [this](float Delta) { ModifyAccumulatedPercentMovementSpeed(Delta); } }
	};

	if (ModifyPercentFunctions.Contains(StatType))
	{
		ModifyPercentFunctions[StatType](PercentChange);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unsupported StatType for ApplyPercentBuff"));
		return -1;
	}

	int32 BuffIndex = ActiveBuffs.Num();
	FActiveBuff NewBuff(BuffIndex, StatType, PercentChange, FTimerHandle());

	GetWorld()->GetTimerManager().SetTimer(NewBuff.TimerHandle, [this, StatType, PercentChange, BuffIndex]()
		{
			if (ModifyPercentFunctions.Contains(StatType))
			{
				ModifyPercentFunctions[StatType](-PercentChange);
			}

			ActiveBuffs.RemoveAll([BuffIndex](const FActiveBuff& Buff) { return Buff.Index == BuffIndex; });
		}, Duration, false);

	ActiveBuffs.Add(NewBuff);

	return BuffIndex;
}


void UStatComponent::RemoveBuff(int32 BuffIndex)
{
	if (BuffIndex < 0 || BuffIndex >= ActiveBuffs.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveBuff: Invalid BuffIndex %d"), BuffIndex);
		return;
	}

	FActiveBuff& BuffToRemove = ActiveBuffs[BuffIndex];

	static const TMap<ECharacterStatType, ModifyFunction> ModifyFlatFunctions = {
		{ ECharacterStatType::MaxHealth,          [this](float Delta) { ModifyAccumulatedFlatMaxHP(Delta); } },
		{ ECharacterStatType::CurrentHealth,      [this](float Delta) { ModifyCurrentHP(Delta); } },
		{ ECharacterStatType::MaxMana,            [this](float Delta) { ModifyAccumulatedFlatMaxMP(Delta); } },
		{ ECharacterStatType::CurrentMana,        [this](float Delta) { ModifyCurrentMP(Delta); } },
		{ ECharacterStatType::HealthRegeneration, [this](float Delta) { ModifyAccumulatedFlatHealthRegeneration(Delta); } },
		{ ECharacterStatType::ManaRegeneration,   [this](float Delta) { ModifyAccumulatedFlatManaRegeneration(Delta); } },
		{ ECharacterStatType::AttackDamage,       [this](float Delta) { ModifyAccumulatedFlatAttackDamage(Delta); } },
		{ ECharacterStatType::AbilityPower,       [this](float Delta) { ModifyAccumulatedFlatAbilityPower(Delta); } },
		{ ECharacterStatType::DefensePower,       [this](float Delta) { ModifyAccumulatedFlatDefensePower(Delta); } },
		{ ECharacterStatType::MagicResistance,    [this](float Delta) { ModifyAccumulatedFlatMagicResistance(Delta); } },
		{ ECharacterStatType::AttackSpeed,        [this](float Delta) { ModifyAccumulatedFlatAttackSpeed(Delta); } },
		{ ECharacterStatType::AbilityHaste,       [this](float Delta) { ModifyAccumulatedFlatAbilityHaste(Delta); } },
		{ ECharacterStatType::CriticalChance,     [this](float Delta) { ModifyAccumulatedFlatCriticalChance(Delta); } },
		{ ECharacterStatType::MovementSpeed,      [this](float Delta) { ModifyAccumulatedFlatMovementSpeed(Delta); } }
	};

	if (ModifyFlatFunctions.Contains(BuffToRemove.StatType))
	{
		ModifyFlatFunctions[BuffToRemove.StatType](-BuffToRemove.ChangeAmount);
	}

	GetWorld()->GetTimerManager().ClearTimer(BuffToRemove.TimerHandle);

	ActiveBuffs.RemoveAt(BuffIndex);
}

void UStatComponent::RemovePercentBuff(int32 BuffIndex)
{
	if (BuffIndex < 0 || BuffIndex >= ActiveBuffs.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemovePercentBuff: Invalid BuffIndex %d"), BuffIndex);
		return;
	}

	FActiveBuff& BuffToRemove = ActiveBuffs[BuffIndex];

	static const TMap<ECharacterStatType, ModifyFunction> ModifyPercentFunctions = {
		{ ECharacterStatType::AttackSpeed,        [this](float Delta) { ModifyAccumulatedPercentAttackSpeed(Delta); } },
		{ ECharacterStatType::MovementSpeed,      [this](float Delta) { ModifyAccumulatedPercentMovementSpeed(Delta); } }
	};

	if (ModifyPercentFunctions.Contains(BuffToRemove.StatType))
	{
		ModifyPercentFunctions[BuffToRemove.StatType](-BuffToRemove.ChangeAmount);
	}

	GetWorld()->GetTimerManager().ClearTimer(BuffToRemove.TimerHandle);

	ActiveBuffs.RemoveAt(BuffIndex);
}



void UStatComponent::OnOutOfCurrentHP_NetMulticast_Implementation()
{
	if (OnOutOfCurrentHP.IsBound())
	{
		OnOutOfCurrentHP.Broadcast();
	}
}

void UStatComponent::OnMaxHPChanged_NetMulticast_Implementation(float InOldMaxHP, float InNewMaxHP)
{
	if (OnMaxHPChanged.IsBound())
	{
		OnMaxHPChanged.Broadcast(InOldMaxHP, InNewMaxHP);
	}
}

void UStatComponent::OnCurrentHPChanged_NetMulticast_Implementation(float InOldCurrentHP, float InNewCurrentHP)
{
	if (OnCurrentHPChanged.IsBound())
	{
		OnCurrentHPChanged.Broadcast(InOldCurrentHP, InNewCurrentHP);
	}
}

void UStatComponent::OnMaxMPChanged_NetMulticast_Implementation(float InOldMaxMP, float InNewMaxMP)
{
	if (OnMaxMPChanged.IsBound())
	{
		OnMaxMPChanged.Broadcast(InOldMaxMP, InNewMaxMP);
	}
}

void UStatComponent::OnCurrentMPChanged_NetMulticast_Implementation(float InOldCurrentMP, float InNewCurrentMP)
{
	if (OnCurrentMPChanged.IsBound())
	{
		OnCurrentMPChanged.Broadcast(InOldCurrentMP, InNewCurrentMP);
	}
}

void UStatComponent::OnMaxEXPChanged_NetMulticast_Implementation(float InOldMaxEXP, float InNewMaxEXP)
{
	if (OnMaxEXPChanged.IsBound())
	{
		OnMaxEXPChanged.Broadcast(InOldMaxEXP, InNewMaxEXP);
	}
}

void UStatComponent::OnCurrentEXPChanged_NetMulticast_Implementation(float InOldCurrentEXP, float InNewCurrentEXP)
{
	if (OnCurrentEXPChanged.IsBound())
	{
		OnCurrentEXPChanged.Broadcast(InOldCurrentEXP, InNewCurrentEXP);
	}
}

void UStatComponent::OnCurrentLevelChanged_NetMulticast_Implementation(int32 InOldCurrentLevel, int32 InNewCurrentLevel)
{
	if (OnCurrentLevelChanged.IsBound())
	{
		OnCurrentLevelChanged.Broadcast(InOldCurrentLevel, InNewCurrentLevel);
	}
}

void UStatComponent::OnHealthRegenerationChanged_NetMulticast_Implementation(float InOldHealthRegeneration, float InNewHealthRegeneration)
{
	if (OnHPRegenChanged.IsBound())
	{
		OnHPRegenChanged.Broadcast(InOldHealthRegeneration, InNewHealthRegeneration);
	}
}

void UStatComponent::OnManaRegenerationChanged_NetMulticast_Implementation(float InOldManaRegeneration, float InNewManaRegeneration)
{
	if (OnMPRegenChanged.IsBound())
	{
		OnMPRegenChanged.Broadcast(InOldManaRegeneration, InNewManaRegeneration);
	}
}

void UStatComponent::OnAttackDamageChanged_NetMulticast_Implementation(float InOldAttackDamage, float InNewAttackDamage)
{
	if (OnAttackDamageChanged.IsBound())
	{
		OnAttackDamageChanged.Broadcast(InOldAttackDamage, InNewAttackDamage);
	}
}

void UStatComponent::OnAbilityPowerChanged_NetMulticast_Implementation(float InOldAbilityPower, float InNewAbilityPower)
{
	if (OnAbilityPowerChanged.IsBound())
	{
		OnAbilityPowerChanged.Broadcast(InOldAbilityPower, InNewAbilityPower);
	}
}

void UStatComponent::OnDefensePowerChanged_NetMulticast_Implementation(float InOldDefensePower, float InNewDefensePower)
{
	if (OnDefensePowerChanged.IsBound())
	{
		OnDefensePowerChanged.Broadcast(InOldDefensePower, InNewDefensePower);
	}
}

void UStatComponent::OnMagicResistanceChanged_NetMulticast_Implementation(float InOldMagicResistance, float InNewMagicResistance)
{
	if (OnMagicResistanceChanged.IsBound())
	{
		OnMagicResistanceChanged.Broadcast(InOldMagicResistance, InNewMagicResistance);
	}
}

void UStatComponent::OnAttackSpeedChanged_NetMulticast_Implementation(float InOldAttackSpeed, float InNewAttackSpeed)
{
	if (OnAttackSpeedChanged.IsBound())
	{
		OnAttackSpeedChanged.Broadcast(InOldAttackSpeed, InNewAttackSpeed);
	}
}

void UStatComponent::OnAbilityHasteChanged_NetMulticast_Implementation(int32 InOldAbilityHaste, int32 InNewAbilityHaste)
{
	if (OnAbilityHasteChanged.IsBound())
	{
		OnAbilityHasteChanged.Broadcast(InOldAbilityHaste, InNewAbilityHaste);
	}
}

void UStatComponent::OnCriticalChanceChanged_NetMulticast_Implementation(int32 InOldCriticalChance, int32 InNewCriticalChance)
{
	if (OnCriticalChanceChanged.IsBound())
	{
		OnCriticalChanceChanged.Broadcast(InOldCriticalChance, InNewCriticalChance);
	}
}

void UStatComponent::OnMovementSpeedChanged_NetMulticast_Implementation(float InOldMovementSpeed, float InNewMovementSpeed)
{
	if (OnMovementSpeedChanged.IsBound())
	{
		OnMovementSpeedChanged.Broadcast(InOldMovementSpeed, InNewMovementSpeed);
	}
}

void UStatComponent::OnRep_CharacterStatReplicated()
{
	if (OnCharacterStatReplicated.IsBound())
	{
		OnCharacterStatReplicated.Broadcast();
	}
}