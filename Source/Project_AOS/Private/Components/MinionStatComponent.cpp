// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MinionStatComponent.h"
#include "Game/AOSGameState.h"
#include "Game/AOSGameInstance.h"
#include "GameFramework/Actor.h"
#include "DataProviders/CharacterDataProviderBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"


UMinionStatComponent::UMinionStatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    ElapsedTime = 0.f;
}

void UMinionStatComponent::InitStatComponent(ICharacterDataProviderInterface* InDataProvider, const FName& InRowName)
{
    if (!InDataProvider)
    {
        UE_LOG(LogTemp, Error, TEXT("UStatComponent::InitStatComponent - DataProvider is null"));
        return;
    }

    DataProvider = InDataProvider;
    RowName = InRowName;

    if (GetWorld())
    {
        AAOSGameState* GameState = Cast<AAOSGameState>(UGameplayStatics::GetGameState(GetWorld()));
        if (GameState)
        {
            ElapsedTime = GameState->GetElapsedTime();
            UpdateStatsBasedOnElapsedTime(RowName);
        }
    }
}

void UMinionStatComponent::UpdateStatsBasedOnElapsedTime(const FName& InRowName)
{
    UAOSGameInstance* AOSGameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!AOSGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("[UMinionStatComponent::UpdateStatsBasedOnElapsedTime] Invalid AOSGameInstance."));
        return;
    }

    const FStatTableRow* StatRow = DataProvider->GetCharacterStat(InRowName, 1);
    if (!StatRow)
    {
        UE_LOG(LogTemp, Error, TEXT("[UMinionStatComponent::UpdateStatsBasedOnElapsedTime] Invalid StatTableRow."));
        return;
    }

    TMap<FString, float> UniqueValues = StatRow->UniqueAttributes;

    const float* HealthIncreasePtr = UniqueValues.Find("HealthIncreaseAmount");
    const float* AdditionalIncreasePtr = UniqueValues.Find("AdditionalHealthAmount");

    const float StartIncreaseTime = 165.0f; // 2:45 in seconds
    const float MaxHealthLimit = 1300.0f;
    const float Interval = 90.0f; // 90 seconds
    const float InitialMaxHP = StatRow->MaxHP;
    const float InitialAttackDamage = StatRow->AttackDamage;
    const float BaseHealthIncrease = (HealthIncreasePtr != nullptr) ? *HealthIncreasePtr : 22.f;
    const float AdditionalIncreasePerInterval = (AdditionalIncreasePtr != nullptr) ? *AdditionalIncreasePtr : 0.3f;

    const float BaseAttackIncrease = 5.0f; // Base attack increase amount

    // 시간 경과 계산
    float TimeSinceStart = FMath::Max(0.0f, ElapsedTime - StartIncreaseTime);
    int IntervalsPassed = static_cast<int>(TimeSinceStart / Interval);

    // 체력 증가량 계산
    float TotalHealthIncrease = 0.0f;
    float TotalAttackIncrease = 0.0f;

    for (int i = 0; i < IntervalsPassed; ++i)
    {
        TotalHealthIncrease += BaseHealthIncrease + (i * AdditionalIncreasePerInterval);
        TotalAttackIncrease += BaseAttackIncrease;
    }

    // 최종 체력과 공격력 계산
    float CurrentMaxHealth = FMath::Min(InitialMaxHP + TotalHealthIncrease, MaxHealthLimit);
    float CurrentAttackDamage = InitialAttackDamage + TotalAttackIncrease;

    // 기본 스탯 설정
    BaseMaxHP = CurrentMaxHealth;
    BaseMaxMP = 0;
    BaseHealthRegeneration = StatRow->HealthRegeneration;
    BaseManaRegeneration = StatRow->ManaRegeneration;
    BaseAttackDamage = CurrentAttackDamage;
    BaseAbilityPower = StatRow->AbilityPower;
    BaseDefensePower = StatRow->DefensePower;
    BaseMagicResistance = StatRow->MagicResistance;
    BaseAttackSpeed = StatRow->AttackSpeed;
    BaseCriticalChance = StatRow->CriticalChance;
    BaseMovementSpeed = StatRow->MovementSpeed;

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

    RecalculateStats();
}

