#include "Components/AbilityStatComponent.h"
#include "DataProviders/CharacterDataProviderBase.h"
#include "Components/StatComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Game/AOSGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Structs/EnumAbilityType.h"		

UAbilityStatComponent::UAbilityStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;
}

void UAbilityStatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacterBase>(GetOwner());
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

void UAbilityStatComponent::InitAbilityStatComponent(ICharacterDataProviderInterface* InDataProvider, UStatComponent* InStatComponent, const FName& InRowName)
{
	if (!InDataProvider)
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityStatComponent::InitAbilityStatComponent - DataProvider is not valid."));
		return;
	}

	if (!InStatComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UAbilityStatComponent::InitAbilityStatComponent - StatComponent is not valid."));
		return;
	}

	RowName = InRowName;
	DataProvider = InDataProvider;
	StatComponent = InStatComponent;

	CooldownTimers.Add(EAbilityID::Ability_Q, FTimerHandle());
	CooldownTimers.Add(EAbilityID::Ability_E, FTimerHandle());
	CooldownTimers.Add(EAbilityID::Ability_R, FTimerHandle());
	CooldownTimers.Add(EAbilityID::Ability_LMB, FTimerHandle());
	CooldownTimers.Add(EAbilityID::Ability_RMB, FTimerHandle());

	// 데이터 제공자를 통해 행을 가져와서 스탯을 초기화합니다.
	InitializeAbilityInformation(EAbilityID::Ability_Q, Ability_Q_Info, RowName);
	InitializeAbilityInformation(EAbilityID::Ability_E, Ability_E_Info, RowName);
	InitializeAbilityInformation(EAbilityID::Ability_R, Ability_R_Info, RowName);
	InitializeAbilityInformation(EAbilityID::Ability_LMB, Ability_LMB_Info, RowName);
	InitializeAbilityInformation(EAbilityID::Ability_RMB, Ability_RMB_Info, RowName);

	UE_LOG(LogTemp, Log, TEXT("UAbilityStatComponent::InitAbilityStatComponent - Initialized abilities for row: %s"), *RowName.ToString());
}


void UAbilityStatComponent::InitializeAbilityInformation(EAbilityID AbilityID, FAbilityDetails& AbilityInfo, const FName& InRowName)
{
	const FAbility* Ability = DataProvider->GetCharacterAbility(InRowName, AbilityID, 1);
	if (Ability)
	{
		AbilityInfo.AbilityID = AbilityID;
		AbilityInfo.Name = Ability->AbilityInformation.Name;
		AbilityInfo.Description = Ability->AbilityInformation.Description;
		AbilityInfo.MaxLevel = Ability->AbilityInformation.MaxLevel;
		AbilityInfo.MaxInstances = Ability->AbilityInformation.MaxInstances;
		AbilityInfo.AbilityType = Ability->AbilityInformation.AbilityType;
		AbilityInfo.AbilityDetection = Ability->AbilityInformation.AbilityDetection;

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

void UAbilityStatComponent::InitializeAbility(EAbilityID AbilityID, FAbilityDetails& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, const int32 InLevel)
{
	int32 Level = FMath::Clamp(InLevel, 1, AbilityInfo.MaxLevel);

	if (Level == 1)
	{
		AbilityInfo.bAbilityReady = true;
		if(AbilityInfo.bCanCastAbility) BroadcastAbilityVisibility_Client(AbilityID, true);
	}

	BroadcastAbilityLevelChanged_Client(AbilityID, Level);

	AbilityStat.Empty();

	for (int InstanceIndex = 1; InstanceIndex <= AbilityInfo.MaxInstances; InstanceIndex++)
	{
		const FAbilityStatTable* CharacterAbilityStat = DataProvider->GetAbilityStatData(RowName, AbilityID, Level, InstanceIndex);
		if (CharacterAbilityStat)
		{
			FAbilityStatTable NewStatTable = *CharacterAbilityStat;

			for (const FAbilityAttribute& Attribute : CharacterAbilityStat->UniqueAttributes)
			{
				NewStatTable.AddUniqueAttribute(Attribute.Key, Attribute.Value);
				UE_LOG(LogTemp, Log, TEXT("Ability %d, Instance %d - Key: %s, Value: %f"), static_cast<int32>(AbilityID), InstanceIndex, *Attribute.Key, Attribute.Value);
			}

			AbilityStat.Emplace(NewStatTable);
			UE_LOG(LogTemp, Log, TEXT("Added stat table for ability %d, Instance %d: Name: %s, CurrentLevel: %d"), static_cast<int32>(AbilityID), InstanceIndex, *NewStatTable.Name, NewStatTable.CurrentLevel);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No stat data found for ability %d, Instance %d, Level %d"), static_cast<int32>(AbilityID), InstanceIndex, Level);
		}
	}

	if (AbilityStat.IsValidIndex(0))
	{
		AbilityInfo.CurrentLevel = AbilityStat[0].CurrentLevel;
		AbilityInfo.InstanceIndex = 1;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability %d initialization failed. No valid ability stats found."), static_cast<int32>(AbilityID));
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
		UseSpecificAbility(Ability_Q_Info, Ability_Q_Stat, Ability_Q_Info.Cooldown, Ability_Q_Info.LastUseTime, CurrentTime, EAbilityID::Ability_Q);
		break;

	case EAbilityID::Ability_E:
		UseSpecificAbility(Ability_E_Info, Ability_E_Stat, Ability_E_Info.Cooldown, Ability_E_Info.LastUseTime, CurrentTime, EAbilityID::Ability_E);
		break;

	case EAbilityID::Ability_R:
		UseSpecificAbility(Ability_R_Info, Ability_R_Stat, Ability_R_Info.Cooldown, Ability_R_Info.LastUseTime, CurrentTime, EAbilityID::Ability_R);
		break;

	case EAbilityID::Ability_LMB:
		UseSpecificAbility(Ability_LMB_Info, Ability_LMB_Stat, Ability_LMB_Info.Cooldown, Ability_LMB_Info.LastUseTime, CurrentTime, EAbilityID::Ability_LMB);
		break;

	case EAbilityID::Ability_RMB:
		UseSpecificAbility(Ability_RMB_Info, Ability_RMB_Stat, Ability_RMB_Info.Cooldown, Ability_RMB_Info.LastUseTime, CurrentTime, EAbilityID::Ability_RMB);
		break;
	}
}

void UAbilityStatComponent::UseSpecificAbility(FAbilityDetails& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, float& Cooldown, float& LastUseTime, float CurrentTime, EAbilityID AbilityID)
{
	if (AbilityInfo.InstanceIndex < AbilityInfo.MaxInstances &&
		(Cooldown <= 0.0f || (AbilityStat[AbilityInfo.InstanceIndex - 1].ReuseDuration > 0.0f && CurrentTime - LastUseTime <= AbilityStat[AbilityInfo.InstanceIndex - 1].ReuseDuration)))
	{
		AbilityInfo.InstanceIndex = FMath::Clamp<int32>(AbilityInfo.InstanceIndex + 1, 1, AbilityInfo.MaxInstances);
		LastUseTime = CurrentTime;

		if (AbilityInfo.InstanceIndex >= AbilityInfo.MaxInstances)
		{
			AbilityInfo.bAbilityReady = false;
		}
		else
		{
			AbilityInfo.bAbilityReady = true;
		}
	}
	else
	{
		AbilityInfo.bAbilityReady = false;
	}
}

bool UAbilityStatComponent::IsAbilityReady(EAbilityID AbilityID) const
{
	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::IsAbilityReady] Ability ID 는 None이 될 수 없습니다."));
		return false;

	case EAbilityID::Ability_Q:
		return Ability_Q_Info.bAbilityReady && Ability_Q_Info.bCanCastAbility;

	case EAbilityID::Ability_E:
		return Ability_E_Info.bAbilityReady && Ability_E_Info.bCanCastAbility;

	case EAbilityID::Ability_R:
		return Ability_R_Info.bAbilityReady && Ability_R_Info.bCanCastAbility;

	case EAbilityID::Ability_LMB:
		return Ability_LMB_Info.bAbilityReady && Ability_LMB_Info.bCanCastAbility;

	case EAbilityID::Ability_RMB:
		return Ability_RMB_Info.bAbilityReady && Ability_RMB_Info.bCanCastAbility;
	}

	return false;
}

void UAbilityStatComponent::BanUseAbilityFewSeconds(float Seconds)
{
	// 기존 타이머가 활성화되어 있는지 확인하고 제거
	if (GetWorld()->GetTimerManager().IsTimerActive(Ability_Ban_Timer))
	{
		GetWorld()->GetTimerManager().ClearTimer(Ability_Ban_Timer);
	}

	// 능력 사용 금지
	BroadcastAbilityVisibility_Client(EAbilityID::Ability_Q, false);
	BroadcastAbilityVisibility_Client(EAbilityID::Ability_E, false);
	BroadcastAbilityVisibility_Client(EAbilityID::Ability_R, false);
	BroadcastAbilityVisibility_Client(EAbilityID::Ability_LMB, false);
	BroadcastAbilityVisibility_Client(EAbilityID::Ability_RMB, false);

	Ability_Q_Info.bAbilityReady = false;
	Ability_E_Info.bAbilityReady = false;
	Ability_R_Info.bAbilityReady = false;
	Ability_LMB_Info.bAbilityReady = false;
	Ability_RMB_Info.bAbilityReady = false;

	// 새로운 타이머를 설정
	GetWorld()->GetTimerManager().SetTimer(
		Ability_Ban_Timer,
		FTimerDelegate::CreateLambda([&]()
			{
				// 타이머 만료 시, 능력 사용 가능하도록 설정
				Ability_Q_Info.bAbilityReady = true;
				Ability_E_Info.bAbilityReady = true;
				Ability_R_Info.bAbilityReady = true;
				Ability_LMB_Info.bAbilityReady = true;
				Ability_RMB_Info.bAbilityReady = true;

				BroadcastAbilityVisibility_Client(EAbilityID::Ability_Q, true);
				BroadcastAbilityVisibility_Client(EAbilityID::Ability_E, true);
				BroadcastAbilityVisibility_Client(EAbilityID::Ability_R, true);
				BroadcastAbilityVisibility_Client(EAbilityID::Ability_LMB, true);
				BroadcastAbilityVisibility_Client(EAbilityID::Ability_RMB, true);
			}),
		Seconds,
		false
	);
}

void UAbilityStatComponent::BanUseAbilityFewSeconds(EAbilityDetection AbilityDetection, float Seconds)
{
	
	
}

void UAbilityStatComponent::StartAbilityCooldown_Implementation(EAbilityID AbilityID)
{
	if (!StatComponent.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::StartAbilityCooldown] StatComponent is not valid."));
		return;
	}

	// Ability cooldown 설정
	int AbilityHaste = StatComponent->GetAbilityHaste();
	float* MaxCooldownPtr = nullptr;
	float* CurrentCooldownPtr = nullptr;
	FTimerHandle& TimerHandle = CooldownTimers.FindOrAdd(AbilityID);

	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::SetAbilityCooldown] Ability ID 는 None이 될 수 없습니다."));
		return;

	case EAbilityID::Ability_Q:
		MaxCooldownPtr = &Ability_Q_Info.MaxCooldown;
		CurrentCooldownPtr = &Ability_Q_Info.Cooldown;
		SetupAbilityCooldown(Ability_Q_Info, Ability_Q_Stat, AbilityHaste, *MaxCooldownPtr, *CurrentCooldownPtr, TimerHandle);
		break;

	case EAbilityID::Ability_E:
		MaxCooldownPtr = &Ability_E_Info.MaxCooldown;
		CurrentCooldownPtr = &Ability_E_Info.Cooldown;
		SetupAbilityCooldown(Ability_E_Info, Ability_E_Stat, AbilityHaste, *MaxCooldownPtr, *CurrentCooldownPtr, TimerHandle);
		break;

	case EAbilityID::Ability_R:
		MaxCooldownPtr = &Ability_R_Info.MaxCooldown;
		CurrentCooldownPtr = &Ability_R_Info.Cooldown;
		SetupAbilityCooldown(Ability_R_Info, Ability_R_Stat, AbilityHaste, *MaxCooldownPtr, *CurrentCooldownPtr, TimerHandle);
		break;

	case EAbilityID::Ability_LMB:
		MaxCooldownPtr = &Ability_LMB_Info.MaxCooldown;
		CurrentCooldownPtr = &Ability_LMB_Info.Cooldown;
		if (MaxCooldownPtr && CurrentCooldownPtr)
		{
			*MaxCooldownPtr = 1.0f / StatComponent->GetAttackSpeed();
			*CurrentCooldownPtr = *MaxCooldownPtr;
		}
		break;

	case EAbilityID::Ability_RMB:
		MaxCooldownPtr = &Ability_RMB_Info.MaxCooldown;
		CurrentCooldownPtr = &Ability_RMB_Info.Cooldown;
		SetupAbilityCooldown(Ability_RMB_Info, Ability_RMB_Stat, AbilityHaste, *MaxCooldownPtr, *CurrentCooldownPtr, TimerHandle);
		break;

	default:
		return;
	}

	// 쿨다운 타이머 시작
	if (MaxCooldownPtr && CurrentCooldownPtr)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, AbilityID, MaxCooldown = MaxCooldownPtr, CurrentCooldown = CurrentCooldownPtr]()
			{
				*CurrentCooldown -= 0.05f;
				if (*CurrentCooldown <= 0)
				{
					HandleAbilityReady(AbilityID);
					*CurrentCooldown = 0;
					GetWorld()->GetTimerManager().ClearTimer(CooldownTimers[AbilityID]);
				}
				NotifyCooldownChanged(AbilityID, *MaxCooldown, *CurrentCooldown);

			}, 0.05f, true, 0.05f);
	}
}

void UAbilityStatComponent::SetupAbilityCooldown(FAbilityDetails& AbilityInfo, const TArray<FAbilityStatTable>& AbilityStat, int AbilityHaste, float& MaxCooldown, float& CurrentCooldown, FTimerHandle& TimerHandle)
{
	uint8 Index = FMath::Clamp(AbilityInfo.InstanceIndex - 1, 0, AbilityInfo.MaxInstances - 1);
	if (AbilityStat.IsValidIndex(Index))
	{
		MaxCooldown = AbilityStat[Index].Cooldown * (100.f / (100.f + AbilityHaste));
		CurrentCooldown = MaxCooldown;
	}
}

void UAbilityStatComponent::HandleAbilityReady(EAbilityID AbilityID)
{
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		Ability_Q_Info.bAbilityReady = true;
		Ability_Q_Info.InstanceIndex = 1;
		break;
	case EAbilityID::Ability_E:
		Ability_E_Info.bAbilityReady = true;
		Ability_E_Info.InstanceIndex = 1;
		break;
	case EAbilityID::Ability_R:
		Ability_R_Info.bAbilityReady = true;
		Ability_R_Info.InstanceIndex = 1;
		break;
	case EAbilityID::Ability_LMB:
		Ability_LMB_Info.bAbilityReady = true;
		Ability_LMB_Info.InstanceIndex = 1;
		break;
	case EAbilityID::Ability_RMB:
		Ability_RMB_Info.bAbilityReady = true;
		Ability_RMB_Info.InstanceIndex = 1;
		break;
	default:
		break;
	}
}

void UAbilityStatComponent::NotifyCooldownChanged(EAbilityID AbilityID, float MaxCooldown, float CurrentCooldown)
{
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		Ability_Q_CooldownChanged_Client(MaxCooldown, CurrentCooldown);
		break;
	case EAbilityID::Ability_E:
		Ability_E_CooldownChanged_Client(MaxCooldown, CurrentCooldown);
		break;
	case EAbilityID::Ability_R:
		Ability_R_CooldownChanged_Client(MaxCooldown, CurrentCooldown);
		break;
	case EAbilityID::Ability_LMB:
		Ability_LMB_CooldownChanged_Client(MaxCooldown, CurrentCooldown);
		break;
	case EAbilityID::Ability_RMB:
		Ability_RMB_CooldownChanged_Client(MaxCooldown, CurrentCooldown);
		break;
	}
}


FAbilityDetails& UAbilityStatComponent::GetAbilityInfomation(EAbilityID AbilityID)
{
	FAbilityDetails EmptyStruct = FAbilityDetails();

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


bool UAbilityStatComponent::GetStatTablesAndIndex(EAbilityID AbilityID, const TArray<FAbilityStatTable>*& OutStatTables, int32& OutArrayIndex) const
{
	switch (AbilityID)
	{
	case EAbilityID::None:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::GetStatTablesAndIndex] Ability ID cannot be None."));
		return false;

	case EAbilityID::Ability_Q:
		OutArrayIndex = FMath::Clamp<int32>(Ability_Q_Info.InstanceIndex, 0, Ability_Q_Info.MaxInstances - 1);
		OutStatTables = &Ability_Q_Stat;
		break;

	case EAbilityID::Ability_E:
		OutArrayIndex = FMath::Clamp<int32>(Ability_E_Info.InstanceIndex, 0, Ability_E_Info.MaxInstances - 1);
		OutStatTables = &Ability_E_Stat;
		break;

	case EAbilityID::Ability_R:
		OutArrayIndex = FMath::Clamp<int32>(Ability_R_Info.InstanceIndex, 0, Ability_R_Info.MaxInstances - 1);
		OutStatTables = &Ability_R_Stat;
		break;

	case EAbilityID::Ability_LMB:
		OutArrayIndex = FMath::Clamp<int32>(Ability_LMB_Info.InstanceIndex, 0, Ability_LMB_Info.MaxInstances - 1);
		OutStatTables = &Ability_LMB_Stat;
		break;

	case EAbilityID::Ability_RMB:
		OutArrayIndex = FMath::Clamp<int32>(Ability_RMB_Info.InstanceIndex, 0, Ability_RMB_Info.MaxInstances - 1);
		OutStatTables = &Ability_RMB_Stat;
		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("[UAbilityStatComponent::GetStatTablesAndIndex] Unknown Ability ID."));
		return false;
	}

	return true;
}


const FAbilityStatTable& UAbilityStatComponent::GetAbilityStatTable(EAbilityID AbilityID) const
{
	// 기본값 반환을 위한 빈 테이블 생성
	static FAbilityStatTable EmptyTable;
	const TArray<FAbilityStatTable>* StatTables = nullptr;
	int32 ArrayIndex = 0;

	// AbilityID에 따른 StatTable 및 ArrayIndex 설정
	if (!GetStatTablesAndIndex(AbilityID, StatTables, ArrayIndex))
	{
		return EmptyTable;
	}

	// StatTables 및 ArrayIndex 유효성 검사
	if (!StatTables->IsValidIndex(ArrayIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UAbilityStatComponent::GetAbilityStatTable] Valid AbilityStatTable not found. Returning default value. AbilityID: %d, ArrayIndex: %d"), static_cast<int32>(AbilityID), ArrayIndex);
		return EmptyTable;
	}

	const FAbilityStatTable& FoundTable = (*StatTables)[ArrayIndex];
	//UE_LOG(LogTemp, Log, TEXT("Ability Stat Table - Name: %s, CurrentLevel: %d, AttackDamage: %f, AbilityDamage: %f, Range: %f"),
	//	*FoundTable.Name, FoundTable.CurrentLevel, FoundTable.AttackDamage, FoundTable.AbilityDamage, FoundTable.Range);
	return FoundTable;
}



float UAbilityStatComponent::GetUniqueValue(EAbilityID AbilityID, const FString& InKey)
{
	// 능력 정보와 능력 속성 배열에 대한 포인터 정의
	const FAbilityDetails* AbilityInfo = nullptr;
	TArray<FAbilityStatTable>* StatTables = nullptr;

	// AbilityID에 따라 포인터 설정
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		AbilityInfo = &Ability_Q_Info;
		StatTables = &Ability_Q_Stat;
		break;
	case EAbilityID::Ability_E:
		AbilityInfo = &Ability_E_Info;
		StatTables = &Ability_E_Stat;
		break;
	case EAbilityID::Ability_R:
		AbilityInfo = &Ability_R_Info;
		StatTables = &Ability_R_Stat;
		break;
	case EAbilityID::Ability_LMB:
		AbilityInfo = &Ability_LMB_Info;
		StatTables = &Ability_LMB_Stat;
		break;
	case EAbilityID::Ability_RMB:
		AbilityInfo = &Ability_RMB_Info;
		StatTables = &Ability_RMB_Stat;
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("GetUniqueValue: Invalid AbilityID"));
		return 0.f;
	}

	// AbilityInfo와 AbilityStatTables가 유효한지 확인
	if (!AbilityInfo || !StatTables)
	{
		UE_LOG(LogTemp, Error, TEXT("GetUniqueValue: Invalid ability info or attributes"));
		return 0.f;
	}

	// 인덱스를 제한하고 속성 배열 가져오기
	int32 ArrayIndex = FMath::Clamp(AbilityInfo->InstanceIndex, 0, AbilityInfo->MaxInstances - 1);
	const TArray<FAbilityAttribute>& Attributes = (*StatTables)[ArrayIndex].UniqueAttributes;

	// 속성 값을 찾기
	for (const auto& AbilityAttribute : Attributes)
	{
		if (AbilityAttribute.Key.Equals(InKey))
		{
			return AbilityAttribute.Value;
		}
	}

	return 0.f;
}

float UAbilityStatComponent::GetUniqueValue(EAbilityID AbilityID, const FString& InKey, float DefaultValue)
{
	float Value = GetUniqueValue(AbilityID, InKey);
	if (FMath::IsNearlyZero(Value))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetUniqueValue: Key '%s' not found for AbilityID '%d', using default value %f."), *InKey, static_cast<int32>(AbilityID), DefaultValue);
		return DefaultValue;
	}
	return Value;
}

float UAbilityStatComponent::GetAbilityCurrentCooldown() const
{
	return 0.0f;
}

TArray<FAbilityDetails*> UAbilityStatComponent::GetAbilityDetailsPtrs()
{
	TArray<FAbilityDetails*> OutDetails;

	if (&Ability_Q_Info != nullptr)
	{
		OutDetails.Add(&Ability_Q_Info);
	}

	if (&Ability_E_Info != nullptr)
	{
		OutDetails.Add(&Ability_E_Info);
	}

	if (&Ability_R_Info != nullptr)
	{
		OutDetails.Add(&Ability_R_Info);
	}

	if (&Ability_LMB_Info != nullptr)
	{
		OutDetails.Add(&Ability_LMB_Info);
	}

	if (&Ability_RMB_Info != nullptr)
	{
		OutDetails.Add(&Ability_RMB_Info);
	}

	return OutDetails;
}

void UAbilityStatComponent::UpdateAbilityUpgradeStatus(EAbilityID AbilityID, FAbilityDetails& AbilityInfo, int32 InNewCurrentLevel)
{
	if (AbilityInfo.CurrentLevel < AbilityInfo.MaxLevel)
	{
		int32 RequiredLevel = DataProvider->GetAbilityStatData(RowName, AbilityID, AbilityInfo.CurrentLevel + 1, 1)->RequiredLevel;
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
	OnUpgradeWidgetVisibilityChanged.Broadcast(AbilityID, InVisibility);
}

void UAbilityStatComponent::BroadcastAbilityVisibility_Client_Implementation(EAbilityID AbilityID, bool InVisibility)
{
	OnAbilityVisibilityChanged.Broadcast(AbilityID, InVisibility);
}

void UAbilityStatComponent::BroadcastAbilityLevelChanged_Client_Implementation(EAbilityID AbilityID, int InLevel)
{
	OnAbilityLevelChanged.Broadcast(AbilityID, InLevel);
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