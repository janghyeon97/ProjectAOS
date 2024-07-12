#include "Components/AbilityStatComponent.h"
#include "Components/StatComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Game/AOSGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

UAbilityStatComponent::UAbilityStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;
}

void UAbilityStatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AAOSCharacterBase>(GetOwner());
	if (!::IsValid(OwnerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityStatComponent's OwnerCharacter is not Valid!!!"));
	}
}

void UAbilityStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAbilityStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Ability_Q_Info);
	DOREPLIFETIME(ThisClass, Ability_E_Info);
	DOREPLIFETIME(ThisClass, Ability_R_Info);
	DOREPLIFETIME(ThisClass, Ability_LMB_Info);
	DOREPLIFETIME(ThisClass, Ability_RMB_Info);
	DOREPLIFETIME(ThisClass, Ability_Q_Stat);
	DOREPLIFETIME(ThisClass, Ability_E_Stat);
	DOREPLIFETIME(ThisClass, Ability_R_Stat);
	DOREPLIFETIME(ThisClass, Ability_LMB_Stat);
	DOREPLIFETIME(ThisClass, Ability_RMB_Stat);
}

void UAbilityStatComponent::InitAbilityStatComponent(UStatComponent* InStatComponent, const int32 InChampionIndex)
{
	if (!::IsValid(InStatComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityStatComponent::InitAbilityStatComponent() - StatComponent is not valid."));
		return;
	}

	StatComponent = InStatComponent;
	StatComponent->OnCurrentLevelChanged.AddDynamic(this, &ThisClass::UpdateLevelUpUI_Server);
	
	GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!::IsValid(GameInstance))
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityStatComponent::InitAbilityStatComponent() - GameInstance is not valid."));
		return;
	}

	if (InChampionIndex <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityStatComponent::InitAbilityStatComponent() - ChampionIndex is not valid."));
		return;
	}

	ChampionIndex = InChampionIndex;

	if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] UAbilityStatComponent::InitAbilityStatComponent() - ChampionIndex %d"), ChampionIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] UAbilityStatComponent::InitAbilityStatComponent() - ChampionIndex %d"), ChampionIndex);
	}

	if (GameInstance->GetCharacterAbilityStatDataTable(ChampionIndex))
	{
		InitializeAbilityInfomation(EAbilityID::Ability_Q, Ability_Q_Info, ChampionIndex);
		InitializeAbilityInfomation(EAbilityID::Ability_E, Ability_E_Info, ChampionIndex);
		InitializeAbilityInfomation(EAbilityID::Ability_R, Ability_R_Info, ChampionIndex);
		InitializeAbilityInfomation(EAbilityID::Ability_LMB, Ability_LMB_Info, ChampionIndex);
		InitializeAbilityInfomation(EAbilityID::Ability_RMB, Ability_RMB_Info, ChampionIndex);
	}
}

void UAbilityStatComponent::InitializeAbilityInfomation(EAbilityID AbilityID, FAbilityAttributes& AbilityInfo, int32 InChampionIndex)
{
	auto AbilityStruct = GameInstance->GetCharacterAbilityStruct(InChampionIndex, AbilityID, 1);
	if (AbilityStruct != nullptr)
	{
		AbilityInfo.Name = AbilityStruct->AbilityInfomation.Name;
		AbilityInfo.Description = AbilityStruct->AbilityInfomation.Description;
		AbilityInfo.MaxLevel = AbilityStruct->AbilityInfomation.MaxLevel;
		AbilityInfo.MaxInstances = AbilityStruct->AbilityInfomation.MaxInstances;

		UE_LOG(LogTemp, Warning, TEXT("UAbilityStatComponent::InitializeAbility - %s Max Level [%d], Max Instances [%d]"), *AbilityInfo.Name, AbilityInfo.MaxLevel, AbilityInfo.MaxInstances);
	}
}

void UAbilityStatComponent::InitializeAbility(EAbilityID AbilityID, const int32 InLevel)
{
	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::IsAbilityReady] Ability ID 는 None이 될 수 없습니다."));
		break;

	case EAbilityID::Ability_Q:
		InitializeAbility(EAbilityID::Ability_Q, Ability_Q_Info, Ability_Q_Stat, InLevel);
		break;

	case EAbilityID::Ability_E:
		InitializeAbility(EAbilityID::Ability_E, Ability_E_Info, Ability_E_Stat, InLevel);
		break;

	case EAbilityID::Ability_R:
		InitializeAbility(EAbilityID::Ability_R, Ability_R_Info, Ability_R_Stat, InLevel);
		break;

	case EAbilityID::Ability_LMB:
		InitializeAbility(EAbilityID::Ability_LMB, Ability_LMB_Info, Ability_LMB_Stat, InLevel);
		break;

	case EAbilityID::Ability_RMB:
		InitializeAbility(EAbilityID::Ability_RMB, Ability_RMB_Info, Ability_RMB_Stat, InLevel);
		break;
	}
}

void UAbilityStatComponent::InitializeAbility(EAbilityID AbilityID, FAbilityAttributes& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, const int32 InLevel)
{
	int32 Level = FMath::Clamp(InLevel, 1, AbilityInfo.MaxLevel);

	if (Level == 1)
	{
		AbilityInfo.bAblityReady = true;
		BroadcastAbilityVisibility_Client(AbilityID, true);
	}

	BroadcastAbilityLevelChanged_Client(AbilityID, Level);

	AbilityStat.Empty();

	for (int InstanceIndex = 1; InstanceIndex <= AbilityInfo.MaxInstances; InstanceIndex++)
	{
		auto CharacterAbilityStat = GameInstance->GetCharacterAbilityStat(ChampionIndex, AbilityID, Level, InstanceIndex);
		if (CharacterAbilityStat)
		{
			FAbilityStatTable NewStatTable;
			NewStatTable.Name = CharacterAbilityStat->Name;
			NewStatTable.CurrentLevel = CharacterAbilityStat->CurrentLevel;
			NewStatTable.Ability_AttackDamage = CharacterAbilityStat->Ability_AttackDamage;
			NewStatTable.Ability_AbilityPower = CharacterAbilityStat->Ability_AbilityPower;
			NewStatTable.Ability_AD_Ratio = CharacterAbilityStat->Ability_AD_Ratio;
			NewStatTable.Ability_AP_Ratio = CharacterAbilityStat->Ability_AP_Ratio;
			NewStatTable.Cooldown = CharacterAbilityStat->Cooldown;
			NewStatTable.Cost = CharacterAbilityStat->Cost;
			NewStatTable.ReuseDuration = CharacterAbilityStat->ReuseDuration;

			for (const auto& Pair : CharacterAbilityStat->UniqueAttributes)
			{
				NewStatTable.UniqueValueName.AddUnique(Pair.Key);
				NewStatTable.UniqueValue.AddUnique(Pair.Value);

				UE_LOG(LogTemp, Log, TEXT("Key: %s, Value: %f"), *Pair.Key, Pair.Value);
			}

			AbilityStat.Emplace(NewStatTable);
		}
	}

	if (AbilityStat.IsValidIndex(0))
	{
		AbilityInfo.CurrentLevel = AbilityStat[0].CurrentLevel;
		AbilityInfo.InstanceIndex = 1;
	}
}

void UAbilityStatComponent::UseAbility_Implementation(EAbilityID AbilityID, float CurrentTime)
{
	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::UseAbility_Implementation] Ability ID 는 None이 될 수 없습니다."));
		break;

	case EAbilityID::Ability_Q:
		UseSpecificAbility(Ability_Q_Info, Ability_Q_Stat, Ability_Q_Cooldown, Ability_Q_LastUseTime, CurrentTime, EAbilityID::Ability_Q);
		break;

	case EAbilityID::Ability_E:
		UseSpecificAbility(Ability_E_Info, Ability_E_Stat, Ability_E_Cooldown, Ability_E_LastUseTime, CurrentTime, EAbilityID::Ability_E);
		break;

	case EAbilityID::Ability_R:
		UseSpecificAbility(Ability_R_Info, Ability_R_Stat, Ability_R_Cooldown, Ability_R_LastUseTime, CurrentTime, EAbilityID::Ability_R);
		break;

	case EAbilityID::Ability_LMB:
		UseSpecificAbility(Ability_LMB_Info, Ability_LMB_Stat, Ability_LMB_Cooldown, Ability_LMB_LastUseTime, CurrentTime, EAbilityID::Ability_LMB);
		break;

	case EAbilityID::Ability_RMB:
		UseSpecificAbility(Ability_RMB_Info, Ability_RMB_Stat, Ability_RMB_Cooldown, Ability_RMB_LastUseTime, CurrentTime, EAbilityID::Ability_RMB);
		break;
	}
}

void UAbilityStatComponent::UseSpecificAbility(FAbilityAttributes& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, float& Cooldown, float& LastUseTime, float CurrentTime, EAbilityID AbilityID)
{
	if (AbilityInfo.InstanceIndex < AbilityInfo.MaxInstances &&
		(Cooldown <= 0.0f || (AbilityStat[AbilityInfo.InstanceIndex - 1].ReuseDuration > 0.0f && CurrentTime - LastUseTime <= AbilityStat[AbilityInfo.InstanceIndex - 1].ReuseDuration)))
	{
		AbilityInfo.InstanceIndex = FMath::Clamp<int32>(AbilityInfo.InstanceIndex + 1, 1, AbilityInfo.MaxInstances);
		LastUseTime = CurrentTime;

		if (AbilityInfo.InstanceIndex >= AbilityInfo.MaxInstances)
		{
			AbilityInfo.bAblityReady = false;
		}
		else
		{
			AbilityInfo.bAblityReady = true;
		}
	}
	else
	{
		AbilityInfo.bAblityReady = false;
	}
}

bool UAbilityStatComponent::IsAbilityReady(EAbilityID AbilityID)
{
	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::IsAbilityReady] Ability ID 는 None이 될 수 없습니다."));
		return false;

	case EAbilityID::Ability_Q:
		return Ability_Q_Info.bAblityReady;

	case EAbilityID::Ability_E:
		return Ability_E_Info.bAblityReady;

	case EAbilityID::Ability_R:
		return Ability_R_Info.bAblityReady;

	case EAbilityID::Ability_LMB:
		return Ability_LMB_Info.bAblityReady;

	case EAbilityID::Ability_RMB:
		return Ability_RMB_Info.bAblityReady;
	}

	return false;
}

void UAbilityStatComponent::BanUseAbilityFewSeconds(float Seconds)
{
	Ability_Q_Info.bAblityReady = false;
	Ability_E_Info.bAblityReady = false;
	Ability_R_Info.bAblityReady = false;
	Ability_LMB_Info.bAblityReady = false;
	Ability_RMB_Info.bAblityReady = false;

	GetWorld()->GetTimerManager().SetTimer(
		Ability_Ban_Timer,
		FTimerDelegate::CreateLambda([&]()
			{
				Ability_Q_Info.bAblityReady = true;
				Ability_E_Info.bAblityReady = true;
				Ability_R_Info.bAblityReady = true;
				Ability_LMB_Info.bAblityReady = true;
				Ability_RMB_Info.bAblityReady = true;
			}),
		0.1f,
		false,
		Seconds
	);
}

void UAbilityStatComponent::StartAbilityCooldown_Implementation(EAbilityID AbilityID)
{
	uint8 Index = 0;
	int AbilityHaste = StatComponent->GetAbilityHaste();

	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::SetAbilityCooldown] Ability ID 는 None이 될 수 없습니다."));
		break;

	case EAbilityID::Ability_Q:
		Index = FMath::Clamp(Ability_Q_Info.InstanceIndex - 1, 0, Ability_Q_Info.MaxInstances - 1);
		if (Ability_Q_Stat.IsValidIndex(Index))
		{
			Ability_Q_MaxCooldown = Ability_Q_Stat[Index].Cooldown * (100 / 100 + AbilityHaste);
			Ability_Q_Cooldown = Ability_Q_MaxCooldown;
		}

		GetWorld()->GetTimerManager().SetTimer(Ability_Q_Timer, FTimerDelegate::CreateLambda([&]()
			{
				Ability_Q_Cooldown -= 0.05f;
				if (Ability_Q_Cooldown <= 0)
				{
					Ability_Q_Info.bAblityReady = true;
					Ability_Q_Info.InstanceIndex = 1;
					Ability_Q_Cooldown = 0;
					GetWorld()->GetTimerManager().ClearTimer(Ability_Q_Timer);
				}
				Ability_Q_CooldownChanged_Client(Ability_Q_MaxCooldown, Ability_Q_Cooldown);
			}),
			0.05f,
			true,
			0.05f
		);
		break;

	case EAbilityID::Ability_E:
		Index = FMath::Clamp(Ability_E_Info.InstanceIndex - 1, 0, Ability_E_Info.MaxInstances - 1);
		if (Ability_E_Stat.IsValidIndex(Index))
		{
			Ability_E_MaxCooldown = Ability_E_Stat[Index - 1].Cooldown * (100 / 100 + AbilityHaste);
			Ability_E_Cooldown = Ability_E_MaxCooldown;
		}

		GetWorld()->GetTimerManager().SetTimer(Ability_E_Timer, FTimerDelegate::CreateLambda([&]()
			{
				Ability_E_Cooldown -= 0.05f;
				if (Ability_E_Cooldown <= 0)
				{
					Ability_E_Info.bAblityReady = true;
					Ability_E_Info.InstanceIndex = 1;
					Ability_E_Cooldown = 0;
					GetWorld()->GetTimerManager().ClearTimer(Ability_E_Timer);
				}
				Ability_E_CooldownChanged_Client(Ability_E_MaxCooldown, Ability_E_Cooldown);
			}),
			0.05f,
			true,
			0.05f
		);
		break;

	case EAbilityID::Ability_R:
		Index = FMath::Clamp(Ability_R_Info.InstanceIndex - 1, 0, Ability_R_Info.MaxInstances - 1);
		if (Ability_R_Stat.IsValidIndex(Index))
		{
			Ability_R_MaxCooldown = Ability_R_Stat[Index].Cooldown * (100 / 100 + AbilityHaste);
			Ability_R_Cooldown = Ability_R_MaxCooldown;
		}

		GetWorld()->GetTimerManager().SetTimer(Ability_R_Timer, FTimerDelegate::CreateLambda([&]()
			{
				Ability_R_Cooldown -= 0.05f;
				if (Ability_R_Cooldown <= 0)
				{
					Ability_R_Info.bAblityReady = true;
					Ability_R_Info.InstanceIndex = 1;
					Ability_R_Cooldown = 0;
					GetWorld()->GetTimerManager().ClearTimer(Ability_R_Timer);
				}
				Ability_R_CooldownChanged_Client(Ability_R_MaxCooldown, Ability_R_Cooldown);
			}),
			0.05f,
			true,
			0.05f
		);
		break;

	case EAbilityID::Ability_LMB:
		Ability_LMB_MaxCooldown = 1.0f / StatComponent->GetAttackSpeed();
		Ability_LMB_Cooldown = Ability_LMB_MaxCooldown;
		GetWorld()->GetTimerManager().SetTimer(Ability_LMB_Timer, FTimerDelegate::CreateLambda([&]()
			{
				Ability_LMB_Cooldown -= 0.05f;
				if (Ability_LMB_Cooldown <= 0)
				{
					Ability_LMB_Info.bAblityReady = true;
					Ability_LMB_Info.InstanceIndex = 1;
					Ability_LMB_Cooldown = 0;
					GetWorld()->GetTimerManager().ClearTimer(Ability_LMB_Timer);
				}
				Ability_LMB_CooldownChanged_Client(Ability_LMB_MaxCooldown, Ability_LMB_Cooldown);
			}),
			0.05f,
			true,
			0.05f
		);
		break;

	case EAbilityID::Ability_RMB:
		Index = FMath::Clamp(Ability_RMB_Info.InstanceIndex - 1, 0, Ability_RMB_Info.MaxInstances - 1);
		if (Ability_RMB_Stat.IsValidIndex(Index))
		{
			Ability_RMB_MaxCooldown = Ability_RMB_Stat[Index].Cooldown * (100 / 100 + AbilityHaste);
			Ability_RMB_Cooldown = Ability_RMB_MaxCooldown;
		}

		GetWorld()->GetTimerManager().SetTimer(Ability_RMB_Timer, FTimerDelegate::CreateLambda([&]()
			{
				Ability_RMB_Cooldown -= 0.05f;
				if (Ability_RMB_Cooldown <= 0)
				{
					Ability_RMB_Info.bAblityReady = true;
					Ability_RMB_Info.InstanceIndex = 1;
					Ability_RMB_Cooldown = 0;
					GetWorld()->GetTimerManager().ClearTimer(Ability_RMB_Timer);
				}
				Ability_RMB_CooldownChanged_Client(Ability_RMB_MaxCooldown, Ability_RMB_Cooldown);
			}),
			0.05f,
			true,
			0.05f
		);
		break;
	}
}

FAbilityAttributes& UAbilityStatComponent::GetAbilityInfomation(EAbilityID AbilityID)
{
	FAbilityAttributes EmptyStruct = FAbilityAttributes();

	if (AbilityID == EAbilityID::None)
	{
		return EmptyStruct;
	}
	else if (AbilityID == EAbilityID::Ability_Q)
	{
		return Ability_Q_Info;
	}
	else if (AbilityID == EAbilityID::Ability_E)
	{
		return Ability_E_Info;
	}
	else if (AbilityID == EAbilityID::Ability_R)
	{
		return Ability_R_Info;
	}
	else if (AbilityID == EAbilityID::Ability_LMB)
	{
		return Ability_LMB_Info;
	}
	else if (AbilityID == EAbilityID::Ability_RMB)
	{
		return Ability_RMB_Info;
	}

	return EmptyStruct;
}

TArray<FAbilityStatTable>& UAbilityStatComponent::GetAbilityStatTable(EAbilityID AbilityID)
{
	TArray<FAbilityStatTable> EmptyArray = TArray<FAbilityStatTable>();

	if (AbilityID == EAbilityID::None)
	{
		return EmptyArray;
	}
	else if (AbilityID == EAbilityID::Ability_Q)
	{
		return Ability_Q_Stat;
	}
	else if (AbilityID == EAbilityID::Ability_E)
	{
		return Ability_E_Stat;
	}
	else if (AbilityID == EAbilityID::Ability_R)
	{
		return Ability_R_Stat;
	}
	else if (AbilityID == EAbilityID::Ability_LMB)
	{
		return Ability_LMB_Stat;
	}
	else if (AbilityID == EAbilityID::Ability_RMB)
	{
		return Ability_RMB_Stat;
	}

	return EmptyArray;
}

FAbilityStatTable& UAbilityStatComponent::GetAbilityStatTable(EAbilityID AbilityID, int32 InstanceIndex)
{
	FAbilityStatTable EmptyTable;
	int32 ArrayIndex = 0;

	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::IsAbilityReady] Ability ID 는 None이 될 수 없습니다."));
		return EmptyTable;

	case EAbilityID::Ability_Q:
		ArrayIndex = FMath::Clamp<int32>(InstanceIndex, 0, Ability_Q_Info.MaxInstances - 1);
		return Ability_Q_Stat[ArrayIndex];

	case EAbilityID::Ability_E:
		ArrayIndex = FMath::Clamp<int32>(InstanceIndex, 0, Ability_E_Info.MaxInstances - 1);
		return Ability_E_Stat[ArrayIndex];

	case EAbilityID::Ability_R:
		ArrayIndex = FMath::Clamp<int32>(InstanceIndex, 0, Ability_R_Info.MaxInstances - 1);
		return Ability_R_Stat[ArrayIndex];

	case EAbilityID::Ability_LMB:
		ArrayIndex = FMath::Clamp<int32>(InstanceIndex, 0, Ability_LMB_Info.MaxInstances - 1);
		return Ability_LMB_Stat[ArrayIndex];

	case EAbilityID::Ability_RMB:
		ArrayIndex = FMath::Clamp<int32>(InstanceIndex, 0, Ability_RMB_Info.MaxInstances - 1);
		return Ability_RMB_Stat[ArrayIndex];
	}

	return EmptyTable;
}

void UAbilityStatComponent::UpdateAbilityUpgradeStatus(EAbilityID AbilityID, FAbilityAttributes& AbilityInfo, int32 InNewCurrentLevel)
{
	if (AbilityInfo.CurrentLevel < AbilityInfo.MaxLevel)
	{
		int32 RequiredLevel = GameInstance->GetCharacterAbilityStat(ChampionIndex, AbilityID, AbilityInfo.CurrentLevel + 1, 1)->RequiredLevel;
		if (InNewCurrentLevel >= RequiredLevel)
		{
			AbilityInfo.bIsUpgradable = true;
			BroadcastUpgradeWidgetVisibility_Client(AbilityID, true);
		}
		else
		{
			AbilityInfo.bIsUpgradable = false;
			BroadcastUpgradeWidgetVisibility_Client(AbilityID, false);
		}
	}
	else
	{
		AbilityInfo.bIsUpgradable = false;
		BroadcastUpgradeWidgetVisibility_Client(AbilityID, false);
	}
}

void UAbilityStatComponent::UpdateLevelUpUI_Server_Implementation(int32 InOldCurrentLevel, int32 InNewCurrentLevel)
{
	if (!::IsValid(GameInstance))
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityStatComponent::UpdateLevelUpUI_Server_Implementation() - GameInstance is not valid."));
		return;
	}

	UpdateAbilityUpgradeStatus(EAbilityID::Ability_Q, Ability_Q_Info, InNewCurrentLevel);
	UpdateAbilityUpgradeStatus(EAbilityID::Ability_E, Ability_E_Info, InNewCurrentLevel);
	UpdateAbilityUpgradeStatus(EAbilityID::Ability_R, Ability_R_Info, InNewCurrentLevel);
	UpdateAbilityUpgradeStatus(EAbilityID::Ability_LMB, Ability_LMB_Info, InNewCurrentLevel);
	UpdateAbilityUpgradeStatus(EAbilityID::Ability_RMB, Ability_RMB_Info, InNewCurrentLevel);
}

void UAbilityStatComponent::ToggleLevelUpUI_Server_Implementation(bool Visibility)
{
	BroadcastUpgradeWidgetVisibility_Client(EAbilityID::Ability_Q, Visibility);
	BroadcastUpgradeWidgetVisibility_Client(EAbilityID::Ability_E, Visibility);
	BroadcastUpgradeWidgetVisibility_Client(EAbilityID::Ability_R, Visibility);
	BroadcastUpgradeWidgetVisibility_Client(EAbilityID::Ability_LMB, Visibility);
	BroadcastUpgradeWidgetVisibility_Client(EAbilityID::Ability_RMB, Visibility);
}

void UAbilityStatComponent::BroadcastUpgradeWidgetVisibility_Client_Implementation(EAbilityID AbilityID, bool InVisibility)
{
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		OnUpgradeWidgetVisibilityChanged.Broadcast(EAbilityID::Ability_Q, InVisibility);
		break;
	case EAbilityID::Ability_E:
		OnUpgradeWidgetVisibilityChanged.Broadcast(EAbilityID::Ability_E, InVisibility);
		break;
	case EAbilityID::Ability_R:
		OnUpgradeWidgetVisibilityChanged.Broadcast(EAbilityID::Ability_R, InVisibility);
		break;
	case EAbilityID::Ability_LMB:
		OnUpgradeWidgetVisibilityChanged.Broadcast(EAbilityID::Ability_LMB, InVisibility);
		break;
	case EAbilityID::Ability_RMB:
		OnUpgradeWidgetVisibilityChanged.Broadcast(EAbilityID::Ability_RMB, InVisibility);
		break;
	}
}

void UAbilityStatComponent::BroadcastAbilityVisibility_Client_Implementation(EAbilityID AbilityID, bool InVisibility)
{
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		OnAbilityVisibilityChanged.Broadcast(EAbilityID::Ability_Q, InVisibility);
		break;
	case EAbilityID::Ability_E:
		OnAbilityVisibilityChanged.Broadcast(EAbilityID::Ability_E, InVisibility);
		break;
	case EAbilityID::Ability_R:
		OnAbilityVisibilityChanged.Broadcast(EAbilityID::Ability_R, InVisibility);
		break;
	case EAbilityID::Ability_LMB:
		OnAbilityVisibilityChanged.Broadcast(EAbilityID::Ability_LMB, InVisibility);
		break;
	case EAbilityID::Ability_RMB:
		OnAbilityVisibilityChanged.Broadcast(EAbilityID::Ability_RMB, InVisibility);
		break;
	}
}

void UAbilityStatComponent::BroadcastAbilityLevelChanged_Client_Implementation(EAbilityID AbilityID, int InLevel)
{
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		OnAbilityLevelChanged.Broadcast(EAbilityID::Ability_Q, InLevel);
		break;
	case EAbilityID::Ability_E:
		OnAbilityLevelChanged.Broadcast(EAbilityID::Ability_E, InLevel);
		break;
	case EAbilityID::Ability_R:
		OnAbilityLevelChanged.Broadcast(EAbilityID::Ability_R, InLevel);
		break;
	case EAbilityID::Ability_LMB:
		OnAbilityLevelChanged.Broadcast(EAbilityID::Ability_LMB, InLevel);
		break;
	case EAbilityID::Ability_RMB:
		OnAbilityLevelChanged.Broadcast(EAbilityID::Ability_RMB, InLevel);
		break;
	}
}

void UAbilityStatComponent::Ability_Q_CooldownChanged_Client_Implementation(const float InAbility_Q_MaxCooldown, const float InAbility_Q_Cooldown)
{
	if (OnAbilityQCooldownChanged.IsBound())
	{
		OnAbilityQCooldownChanged.Broadcast(InAbility_Q_MaxCooldown, InAbility_Q_Cooldown);
	}
}

void UAbilityStatComponent::Ability_E_CooldownChanged_Client_Implementation(const float InAbility_E_MaxCooldown, const float InAbility_E_Cooldown)
{
	if (OnAbilityECooldownChanged.IsBound())
	{
		OnAbilityECooldownChanged.Broadcast(InAbility_E_MaxCooldown, InAbility_E_Cooldown);
	}
}

void UAbilityStatComponent::Ability_R_CooldownChanged_Client_Implementation(const float InAbility_R_MaxCooldown, const float InAbility_R_Cooldown)
{
	if (OnAbilityRCooldownChanged.IsBound())
	{
		OnAbilityRCooldownChanged.Broadcast(InAbility_R_MaxCooldown, InAbility_R_Cooldown);
	}
}

void UAbilityStatComponent::Ability_LMB_CooldownChanged_Client_Implementation(const float InAbility_LMB_MaxCooldown, const float InAbility_LMB_Cooldown)
{
	if (OnAbilityLMBCooldownChanged.IsBound())
	{
		OnAbilityLMBCooldownChanged.Broadcast(InAbility_LMB_MaxCooldown, InAbility_LMB_Cooldown);
	}
}

void UAbilityStatComponent::Ability_RMB_CooldownChanged_Client_Implementation(const float InAbility_RMB_MaxCooldown, const float InAbility_RMB_Cooldown)
{
	if (OnAbilityRMBCooldownChanged.IsBound())
	{
		OnAbilityRMBCooldownChanged.Broadcast(InAbility_RMB_MaxCooldown, InAbility_RMB_Cooldown);
	}
}

void UAbilityStatComponent::OnHUDInitializationCompleted_Server_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("UAbilityStatComponent::OnHUDInitializationCompleted_Server"));
	InitializeAbility(EAbilityID::Ability_LMB, 1);
}