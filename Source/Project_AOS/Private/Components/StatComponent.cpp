#include "Components/StatComponent.h"
#include "Characters/CharacterBase.h"
#include "Game/AOSGameInstance.h"
#include "Game/AOSPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

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

void UStatComponent::ForceReplication()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		// 리플리케이션 강제 트리거
		GetOwner()->ForceNetUpdate();
	}
}

void UStatComponent::InitializeStatComponent(const int32 InChampionIndex)
{
	GameInstance = Cast<UAOSGameInstance>(GetWorld()->GetGameInstance());
	if (::IsValid(GameInstance))
	{
		ChampionIndex = InChampionIndex;
		if (GameInstance->GetCampionsListTableRow(ChampionIndex)->StatTable != nullptr)
		{
			int32 NewMaxHP = GameInstance->GetCharacterStat(ChampionIndex, 1)->MaxHP;
			int32 NewMaxMP = GameInstance->GetCharacterStat(ChampionIndex, 1)->MaxMP;
			int32 NewMaxEXP = GameInstance->GetCharacterStat(ChampionIndex, 1)->MaxEXP;
			float NewHealthRegeneration = GameInstance->GetCharacterStat(ChampionIndex, 1)->HealthRegeneration;
			float NewManaRegeneration = GameInstance->GetCharacterStat(ChampionIndex, 1)->ManaRegeneration;
			float NewAttackDamage = GameInstance->GetCharacterStat(ChampionIndex, 1)->AttackDamage;
			float NewDefensePower = GameInstance->GetCharacterStat(ChampionIndex, 1)->DefensePower;
			float NewMagicResistance = GameInstance->GetCharacterStat(ChampionIndex, 1)->MagicResistance;
			float NewAttackSpeed = GameInstance->GetCharacterStat(ChampionIndex, 1)->AttackSpeed;
			float NewMovementSpeed = GameInstance->GetCharacterStat(ChampionIndex, 1)->MovementSpeed;

			SetMaxHP(NewMaxHP);
			SetCurrentHP(MaxHP);

			SetMaxMP(NewMaxMP);
			SetCurrentMP(MaxMP);

			SetMaxEXP(NewMaxEXP);
			SetCurrentEXP(0);

			SetHealthRegeneration(NewHealthRegeneration);
			SetManaRegeneration(NewManaRegeneration);
			SetAttackDamage(NewAttackDamage);
			SetAbilityPower(0);
			SetDefensePower(NewDefensePower);
			SetMagicResistance(NewMagicResistance);
			SetAttackSpeed(NewAttackSpeed);
			SetMovementSpeed(NewMovementSpeed);
		}
	}
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
}

#pragma region Setter
void UStatComponent::SetMaxHP(float InMaxHP)
{
	UE_LOG(LogTemp, Log, TEXT("Change MaxHealth :: %f -> %f"), MaxHP, InMaxHP);

	float NewMaxHP = FMath::Clamp<float>(InMaxHP, 0, 99999.f);
	OnMaxHPChanged_NetMulticast(MaxHP, NewMaxHP);
	MaxHP = NewMaxHP;
}

void UStatComponent::SetCurrentHP(float InCurrentHP)
{
	UE_LOG(LogTemp, Log, TEXT("Change CurrentHealth :: %f -> %f"), CurrentHP, InCurrentHP);

	float NewCurrentHP = FMath::Clamp<float>(InCurrentHP, 0, MaxHP);
	OnCurrentHPChanged_NetMulticast(CurrentHP, NewCurrentHP);
	CurrentHP = NewCurrentHP;

	if (CurrentHP < KINDA_SMALL_NUMBER)
	{
		OnOutOfCurrentHP_NetMulticast();

		if (OnHPEqualsMaxHP.IsBound()) OnHPEqualsMaxHP.Broadcast();
		if (OnMPEqualsMaxMP.IsBound()) OnMPEqualsMaxMP.Broadcast();

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
	UE_LOG(LogTemp, Log, TEXT("Change MaxResource :: %f -> %f"), MaxMP, InMaxMP);

	float NewMaxMP = FMath::Clamp<float>(InMaxMP, 0.f, 99999.f);
	OnMaxMPChanged_NetMulticast(MaxMP, NewMaxMP);
	MaxMP = NewMaxMP;
}

void UStatComponent::SetCurrentMP(float InCurrentMP)
{
	UE_LOG(LogTemp, Log, TEXT("Change CurrentResource :: %f -> %f"), CurrentMP, InCurrentMP);

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
	UE_LOG(LogTemp, Log, TEXT("Change CurrentEXP :: %f -> %f"), CurrentEXP, InCurrentEXP);

	if (CurrentLevel < 18)
	{
		if (MaxEXP - KINDA_SMALL_NUMBER < InCurrentEXP)
		{
			CurrentEXP = InCurrentEXP - MaxEXP;
			SetCurrentLevel(FMath::Clamp<int32>(GetCurrentLevel() + 1, 1, MaxLevel));

		}
		else
		{
			CurrentEXP = FMath::Clamp<float>(InCurrentEXP, 0.f, MaxEXP);
		}

		OnCurrentEXPChanged_NetMulticast(CurrentEXP, InCurrentEXP);
	}
	else
	{
		return;
	}
}

void UStatComponent::SetCurrentLevel(int32 InCurrentLevel)
{
	UE_LOG(LogTemp, Log, TEXT("Set CurrentLevel :: %d -> %d"), CurrentLevel, InCurrentLevel);

	float NewCurrentLevel = FMath::Clamp<int32>(InCurrentLevel, 1, MaxLevel);
	
	/*if (OnCurrentLevelChanged.IsBound())
	{
		OnCurrentLevelChanged.Broadcast(CurrentLevel, NewCurrentLevel);
	}*/

	OnCurrentLevelChanged_NetMulticast(CurrentLevel, NewCurrentLevel);

	SetMaxHP(MaxHP + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->MaxHP - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->MaxHP));
	SetCurrentHP(CurrentHP + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->MaxHP - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->MaxHP));
	SetMaxMP(MaxMP + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->MaxMP - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->MaxMP));
	SetCurrentMP(CurrentMP + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->MaxMP - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->MaxMP));
	SetMaxEXP(GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->MaxEXP);
	SetCurrentEXP(CurrentEXP);
	SetHealthRegeneration(HealthRegeneration + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->HealthRegeneration - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->HealthRegeneration));
	SetManaRegeneration(ManaRegeneration + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->ManaRegeneration - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->ManaRegeneration));
	SetAttackDamage(AttackDamage + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->AttackDamage - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->AttackDamage));
	SetDefensePower(DefensePower + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->DefensePower - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->DefensePower));
	SetMagicResistance(MagicResistance + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->MagicResistance - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->MagicResistance));
	SetAttackSpeed(AttackSpeed + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->AttackSpeed - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->AttackSpeed));
	SetMovementSpeed(MovementSpeed + (GameInstance->GetCharacterStat(ChampionIndex, NewCurrentLevel)->MovementSpeed - GameInstance->GetCharacterStat(ChampionIndex, CurrentLevel)->MovementSpeed));

	CurrentLevel = NewCurrentLevel;
}

void UStatComponent::SetHealthRegeneration(float InHealthRegeneration)
{
	UE_LOG(LogTemp, Log, TEXT("Change HealthRegeneration :: %f -> %f"), HealthRegeneration, InHealthRegeneration);

	float NewHealthRegeneration = FMath::Clamp<float>(InHealthRegeneration, 0, 9999.f);
	OnHealthRegenerationChanged_NetMulticast(HealthRegeneration, NewHealthRegeneration);
	HealthRegeneration = NewHealthRegeneration;
}

void UStatComponent::SetManaRegeneration(float InManaRegeneration)
{
	UE_LOG(LogTemp, Log, TEXT("Change ManaRegeneration :: %f -> %f"), ManaRegeneration, InManaRegeneration);

	float NewManaRegeneration = FMath::Clamp<float>(InManaRegeneration, 0, 9999.f);
	OnManaRegenerationChanged_NetMulticast(ManaRegeneration, NewManaRegeneration);
	ManaRegeneration = NewManaRegeneration;
}

void UStatComponent::SetAttackDamage(float InAttackDamage)
{
	UE_LOG(LogTemp, Log, TEXT("Change AttackDamage :: %f -> %f"), AttackDamage, InAttackDamage);

	float NewAttackDamage = FMath::Clamp<float>(InAttackDamage, 0, 99999.f);
	OnAttackDamageChanged_NetMulticast(AttackDamage, NewAttackDamage);
	AttackDamage = NewAttackDamage;
}

void UStatComponent::SetAbilityPower(float InAbilityPower)
{
	UE_LOG(LogTemp, Log, TEXT("Change AbilityPower :: %f -> %f"), AbilityPower, InAbilityPower);

	float NewAbilityPower = FMath::Clamp<float>(InAbilityPower, 0, 99999.f);
	OnAbilityPowerChanged_NetMulticast(AbilityPower, NewAbilityPower);
	AbilityPower = NewAbilityPower;
}

void UStatComponent::SetDefensePower(float InDefensePower)
{
	UE_LOG(LogTemp, Log, TEXT("Change DefensePower :: %f -> %f"), DefensePower, InDefensePower);

	float NewDefensePower = FMath::Clamp<float>(InDefensePower, 0, 9999.f);
	OnDefensePowerChanged_NetMulticast(DefensePower, NewDefensePower);
	DefensePower = NewDefensePower;
}

void UStatComponent::SetMagicResistance(float InMagicResistance)
{
	UE_LOG(LogTemp, Log, TEXT("Change MagicResistance :: %f -> %f"), MagicResistance, InMagicResistance);

	float NewMagicResistance = FMath::Clamp<float>(InMagicResistance, 0, 9999.f);
	OnMagicResistanceChanged_NetMulticast(MagicResistance, NewMagicResistance);
	MagicResistance = NewMagicResistance;
}

void UStatComponent::SetAbilityHaste(int32 InAbilityHaste)
{
	UE_LOG(LogTemp, Log, TEXT("Change AbilityHaste :: %d -> %d"), AbilityHaste, InAbilityHaste);

	int32 NewAbilityHaste = FMath::Clamp<int32>(InAbilityHaste, 0, 300.f);
	OnAbilityHasteChanged_NetMulticast(AbilityHaste, NewAbilityHaste);
	AbilityHaste = NewAbilityHaste;
}

void UStatComponent::SetAttackSpeed(float InAttackSpeed)
{
	UE_LOG(LogTemp, Log, TEXT("Change AttackSpeed :: %f -> %f"), AttackSpeed, InAttackSpeed);

	float NewAttackSpeed = FMath::Clamp<float>(InAttackSpeed, 0, 2.5f);
	OnAttackSpeedChanged_NetMulticast(AttackSpeed, NewAttackSpeed);
	AttackSpeed = NewAttackSpeed;
}

void UStatComponent::SetCriticalChance(int32 InCriticalChance)
{
	UE_LOG(LogTemp, Log, TEXT("Change CriticalChance :: %d -> %d"), CriticalChance, InCriticalChance);

	int32 NewCriticalChance = FMath::Clamp<int32>(InCriticalChance, 0, 100);
	OnCriticalChanceChanged_NetMulticast(CriticalChance, NewCriticalChance);
	CriticalChance = NewCriticalChance;
}

void UStatComponent::SetMovementSpeed(float InMovementSpeed)
{
	UE_LOG(LogTemp, Log, TEXT("Change MovementSpeed :: %f -> %f"), MovementSpeed, InMovementSpeed);

	float NewMovementSpeed = FMath::Clamp<float>(InMovementSpeed, 0, 9999.f);
	OnMovementSpeedChanged_NetMulticast(MovementSpeed, NewMovementSpeed);
	MovementSpeed = NewMovementSpeed;
}
#pragma endregion

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