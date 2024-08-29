// Fill out your copyright notice in the Description page of Project Settings.


#include "DataProviders/MinionDataProvider.h"
#include "Game/AOSGameInstance.h"
#include "Structs/CharacterResources.h"

void UMinionDataProvider::Init(UDataTable* InDataTable)
{
	MinionDataTable = InDataTable;
}

const UDataTable* UMinionDataProvider::GetDataTable() const
{
    return MinionDataTable;
}

const UDataTable* UMinionDataProvider::GetStatDataTable(const FName& RowName) const
{
	if (!::IsValid(MinionDataTable))
	{
		return nullptr;
	}

	const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
	if (!MinionDataTableRow)
	{
		return nullptr;
	}

	const UDataTable* StatTable = MinionDataTableRow->StatTable;
	if (::IsValid(StatTable) == false)
	{
		return nullptr;
	}

	return StatTable;
}

const FStatTableRow* UMinionDataProvider::GetCharacterStat(const FName& RowName, int32 InLevel) const
{
    if (::IsValid(MinionDataTable) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStat] MinionDataTable is invalid."));
        return nullptr;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStat] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    const UDataTable* StatTable = MinionDataTableRow->StatTable;
    if (::IsValid(StatTable) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStat] StatTable is invalid for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    uint8 ClampedLevel = FMath::Clamp<uint8>(InLevel, 1, 18);

    FStatTableRow* StatRow = StatTable->FindRow<FStatTableRow>(FName(*FString::FromInt(ClampedLevel)), TEXT(""));
    if (!StatRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStat] Failed to find StatRow for Level: %d in StatTable for RowName: %s"), ClampedLevel, *RowName.ToString());
        return nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("[UMinionDataProvider::GetCharacterStat] Successfully found StatRow for RowName: %s at Level: %d"), *RowName.ToString(), ClampedLevel);
    return StatRow;
}

const UDataTable* UMinionDataProvider::GetAbilityStatDataTable(const FName& RowName) const
{
    if (::IsValid(MinionDataTable) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatDataTable] ChampionsList is invalid."));
        return nullptr;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatDataTable] Failed to find ChampionsListRow for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    const UDataTable* AbilityStatTable = MinionDataTableRow->AbilityStatTable;
    if (::IsValid(AbilityStatTable) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatDataTable] AbilityStatTable is invalid for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("[UMinionDataProvider::GetAbilityStatDataTable] Successfully found AbilityStatTable for RowName: %s"), *RowName.ToString());
    return AbilityStatTable;
}

const FAbility* UMinionDataProvider::GetCharacterAbility(const FName& RowName, EAbilityID AbilityID, int32 Level) const
{
    if (!::IsValid(MinionDataTable))
    {
        return nullptr;
    }

    // ChampionDataTable에서 RowName에 해당하는 행을 찾습니다.
    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        return nullptr;
    }

    UDataTable* AbilityStatTable = MinionDataTableRow->AbilityStatTable;
    if (!::IsValid(AbilityStatTable))
    {
        return nullptr;
    }

    // 최대 레벨 가져오기
    auto GetMaxLevel = [&](const FAbilityStatTableRow* StatRow) -> uint8 {
        switch (AbilityID)
        {
        case EAbilityID::Ability_Q: return StatRow->Ability_Q.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_E: return StatRow->Ability_E.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_R: return StatRow->Ability_R.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_LMB: return StatRow->Ability_LMB.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_RMB: return StatRow->Ability_RMB.AbilityInformation.MaxLevel;
        default: return 0;
        }
        };

    // 최대 레벨을 가져올 행
    const FAbilityStatTableRow* StatRow = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!StatRow)
    {
        return nullptr;
    }

    uint8 MaxLevel = GetMaxLevel(StatRow);
    uint8 ClampedLevel = FMath::Clamp<uint8>(Level, 1, MaxLevel);

    // 능력 정보 가져오기
    auto GetAbility = [&](const FAbilityStatTableRow* StatRow) -> const FAbility* {
        switch (AbilityID)
        {
        case EAbilityID::Ability_Q: return &StatRow->Ability_Q;
        case EAbilityID::Ability_E: return &StatRow->Ability_E;
        case EAbilityID::Ability_R: return &StatRow->Ability_R;
        case EAbilityID::Ability_LMB: return &StatRow->Ability_LMB;
        case EAbilityID::Ability_RMB: return &StatRow->Ability_RMB;
        default: return nullptr;
        }
        };

    // 실제 레벨에 해당하는 능력 정보 가져오기
    const FAbilityStatTableRow* ClampedStatRow = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(ClampedLevel)), TEXT(""));
    if (!ClampedStatRow)
    {
        return nullptr;
    }

    return GetAbility(ClampedStatRow);
}

const FAbilityInformation* UMinionDataProvider::GetAbilityInfo(const FName& RowName, EAbilityID AbilityID, int32 Level) const
{
    if (!::IsValid(MinionDataTable))
    {
        return nullptr;
    }

    // ChampionDataTable에서 RowName에 해당하는 행을 찾습니다.
    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        return nullptr;
    }

    UDataTable* AbilityStatTable = MinionDataTableRow->AbilityStatTable;
    if (!::IsValid(AbilityStatTable))
    {
        return nullptr;
    }

    // 최대 레벨 가져오기
    auto GetMaxLevel = [&](const FAbilityStatTableRow* StatRow) -> uint8 {
        switch (AbilityID)
        {
        case EAbilityID::Ability_Q: return StatRow->Ability_Q.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_E: return StatRow->Ability_E.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_R: return StatRow->Ability_R.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_LMB: return StatRow->Ability_LMB.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_RMB: return StatRow->Ability_RMB.AbilityInformation.MaxLevel;
        default: return 0;
        }
        };

    // 최대 레벨을 가져올 행
    const FAbilityStatTableRow* StatRow = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!StatRow)
    {
        return nullptr;
    }

    uint8 MaxLevel = GetMaxLevel(StatRow);
    uint8 ClampedLevel = FMath::Clamp<uint8>(Level, 1, MaxLevel);

    // 능력 정보 가져오기
    auto GetAbilityInformation = [&](const FAbilityStatTableRow* StatRow) -> const FAbilityInformation* {
        switch (AbilityID)
        {
        case EAbilityID::Ability_Q: return &StatRow->Ability_Q.AbilityInformation;
        case EAbilityID::Ability_E: return &StatRow->Ability_E.AbilityInformation;
        case EAbilityID::Ability_R: return &StatRow->Ability_R.AbilityInformation;
        case EAbilityID::Ability_LMB: return &StatRow->Ability_LMB.AbilityInformation;
        case EAbilityID::Ability_RMB: return &StatRow->Ability_RMB.AbilityInformation;
        default: return nullptr;
        }
        };

    // 실제 레벨에 해당하는 능력 정보 가져오기
    const FAbilityStatTableRow* ClampedStatRow = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(ClampedLevel)), TEXT(""));
    if (!ClampedStatRow)
    {
        return nullptr;
    }

    return GetAbilityInformation(ClampedStatRow);
}

const FAbilityStatTable* UMinionDataProvider::GetAbilityStatData(const FName& RowName, EAbilityID AbilityID, int32 Level, int32 InstanceIndex) const
{
    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatData] MinionDataTable is invalid."));
        return nullptr;
    }

    // MinionDataTable에서 RowName에 해당하는 행을 찾습니다.
    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatData] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    UDataTable* AbilityStatTable = MinionDataTableRow->AbilityStatTable;
    if (!::IsValid(AbilityStatTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatData] AbilityStatTable is invalid for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    // 최대 레벨 가져오기
    auto GetMaxLevel = [&](const FAbilityStatTableRow* StatRow) -> uint8 {
        switch (AbilityID)
        {
        case EAbilityID::Ability_Q: return StatRow->Ability_Q.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_E: return StatRow->Ability_E.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_R: return StatRow->Ability_R.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_LMB: return StatRow->Ability_LMB.AbilityInformation.MaxLevel;
        case EAbilityID::Ability_RMB: return StatRow->Ability_RMB.AbilityInformation.MaxLevel;
        default: return 0;
        }
        };

    // 최대 레벨을 가져올 행
    const FAbilityStatTableRow* StatRow = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!StatRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatData] Failed to find StatRow for Level: 1 in AbilityStatTable for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    uint8 MaxLevel = GetMaxLevel(StatRow);
    uint8 ClampedLevel = FMath::Clamp<uint8>(Level, 1, MaxLevel);

    // 능력 정보 가져오기
    auto GetAbility = [&](const FAbilityStatTableRow* StatRow) -> const FAbility* {
        switch (AbilityID)
        {
        case EAbilityID::Ability_Q: return &StatRow->Ability_Q;
        case EAbilityID::Ability_E: return &StatRow->Ability_E;
        case EAbilityID::Ability_R: return &StatRow->Ability_R;
        case EAbilityID::Ability_LMB: return &StatRow->Ability_LMB;
        case EAbilityID::Ability_RMB: return &StatRow->Ability_RMB;
        default: return nullptr;
        }
        };

    // 실제 레벨에 해당하는 능력 정보 가져오기
    const FAbilityStatTableRow* ClampedStatRow = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(ClampedLevel)), TEXT(""));
    if (!ClampedStatRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatData] Failed to find StatRow for Level: %d in AbilityStatTable for RowName: %s"), ClampedLevel, *RowName.ToString());
        return nullptr;
    }

    const FAbility* Ability = GetAbility(ClampedStatRow);
    if (!Ability)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatData] Failed to get Ability for RowName: %s at Level: %d"), *RowName.ToString(), ClampedLevel);
        return nullptr;
    }

    int32 InstanceLevel = FMath::Clamp<int32>(InstanceIndex, 1, Ability->AbilityInformation.MaxInstances);
    if (Ability->AbilityStatInformation.IsValidIndex(InstanceLevel - 1))
    {
        return &Ability->AbilityStatInformation[InstanceLevel - 1];
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetAbilityStatData] Invalid InstanceLevel: %d for RowName: %s at Level: %d"), InstanceLevel, *RowName.ToString(), ClampedLevel);
        return nullptr;
    }
}

const UDataTable* UMinionDataProvider::GetCharacterResourcesTable(const FName& RowName) const
{
    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterResourcesTable] MinionDataTable is invalid."));
        return nullptr;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterResourcesTable] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    UDataTable* ResourcesTable = MinionDataTableRow->ResourcesTable;
    if (!::IsValid(ResourcesTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterResourcesTable] ResourcesTable is invalid for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    return ResourcesTable;
}

const TArray<FCharacterAnimationAttribute>& UMinionDataProvider::GetCharacterAnimMontages(const FName& RowName) const
{
    static const TArray<FCharacterAnimationAttribute> EmptyMontages;

    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterAnimMontages] MinionDataTable is invalid."));
        return EmptyMontages;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterAnimMontages] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return EmptyMontages;
    }

    UDataTable* ResourcesTable = MinionDataTableRow->ResourcesTable;
    if (!::IsValid(ResourcesTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterAnimMontages] ResourcesTable is invalid for RowName: %s"), *RowName.ToString());
        return EmptyMontages;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterAnimMontages] Failed to find DataRow in ResourcesTable for RowName: %s"), *RowName.ToString());
        return EmptyMontages;
    }

    return DataRow->GameplayMontages;
}

/** 특정 행 이름에 해당하는 캐릭터 파티클 효과 목록을 반환합니다. */
const TArray<FCharacterParticleEffectAttribute>& UMinionDataProvider::GetCharacterParticleEffects(const FName& RowName) const
{
    static const TArray<FCharacterParticleEffectAttribute> EmptyParticles;

    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticleEffects] MinionDataTable is invalid."));
        return EmptyParticles;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticleEffects] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return EmptyParticles;
    }

    UDataTable* ResourcesTable = MinionDataTableRow->ResourcesTable;
    if (!::IsValid(ResourcesTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticleEffects] ResourcesTable is invalid for RowName: %s"), *RowName.ToString());
        return EmptyParticles;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticleEffects] Failed to find DataRow in ResourcesTable for RowName: %s"), *RowName.ToString());
        return EmptyParticles;
    }

    return DataRow->GameplayParticles;
}

/** 특정 행 이름에 해당하는 캐릭터 스태틱 메시 목록을 반환합니다. */
const TArray<FCharacterStaticMeshAttribute>& UMinionDataProvider::GetCharacterStaticMeshes(const FName& RowName) const
{
    static const TArray<FCharacterStaticMeshAttribute> EmptyMeshes;

    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStaticMeshes] MinionDataTable is invalid."));
        return EmptyMeshes;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStaticMeshes] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return EmptyMeshes;
    }

    UDataTable* ResourcesTable = MinionDataTableRow->ResourcesTable;
    if (!::IsValid(ResourcesTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStaticMeshes] ResourcesTable is invalid for RowName: %s"), *RowName.ToString());
        return EmptyMeshes;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterStaticMeshes] Failed to find DataRow in ResourcesTable for RowName: %s"), *RowName.ToString());
        return EmptyMeshes;
    }

    return DataRow->GameplayMeshes;
}

/** 특정 행 이름에 해당하는 캐릭터 몽타주 맵을 반환합니다. */
const TMap<FName, UAnimMontage*> UMinionDataProvider::GetCharacterMontagesMap(const FName& RowName) const
{
    static const TMap<FName, UAnimMontage*> EmptyMontagesMap;

    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMontagesMap] MinionDataTable is invalid."));
        return EmptyMontagesMap;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMontagesMap] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return EmptyMontagesMap;
    }

    UDataTable* ResourcesTable = MinionDataTableRow->ResourcesTable;
    if (!::IsValid(ResourcesTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMontagesMap] ResourcesTable is invalid for RowName: %s"), *RowName.ToString());
        return EmptyMontagesMap;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMontagesMap] Failed to find DataRow in ResourcesTable for RowName: %s"), *RowName.ToString());
        return EmptyMontagesMap;
    }

    return DataRow->GetGamePlayMontagesMap();
}

/** 특정 행 이름에 해당하는 캐릭터 파티클 맵을 반환합니다. */
const TMap<FName, UParticleSystem*> UMinionDataProvider::GetCharacterParticlesMap(const FName& RowName) const
{
    static const TMap<FName, UParticleSystem*> EmptyParticlesMap;

    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticlesMap] MinionDataTable is invalid."));
        return EmptyParticlesMap;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticlesMap] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return EmptyParticlesMap;
    }

    UDataTable* ResourcesTable = MinionDataTableRow->ResourcesTable;
    if (!::IsValid(ResourcesTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticlesMap] ResourcesTable is invalid for RowName: %s"), *RowName.ToString());
        return EmptyParticlesMap;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterParticlesMap] Failed to find DataRow in ResourcesTable for RowName: %s"), *RowName.ToString());
        return EmptyParticlesMap;
    }

    return DataRow->GetGamePlayParticlesMap();
}

const TMap<FName, UStaticMesh*> UMinionDataProvider::GetCharacterMeshesMap(const FName& RowName) const
{
    static const TMap<FName, UStaticMesh*> EmptyMeshesMap;

    if (!::IsValid(MinionDataTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMeshesMap] MinionDataTable is invalid."));
        return EmptyMeshesMap;
    }

    const FMinionDataTableRow* MinionDataTableRow = MinionDataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
    if (!MinionDataTableRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMeshesMap] Failed to find MinionDataTableRow for RowName: %s"), *RowName.ToString());
        return EmptyMeshesMap;
    }

    UDataTable* ResourcesTable = MinionDataTableRow->ResourcesTable;
    if (!::IsValid(ResourcesTable))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMeshesMap] ResourcesTable is invalid for RowName: %s"), *RowName.ToString());
        return EmptyMeshesMap;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UMinionDataProvider::GetCharacterMeshesMap] Failed to find DataRow in ResourcesTable for RowName: %s"), *RowName.ToString());
        return EmptyMeshesMap;
    }

    return DataRow->GetGamePlayMeshesMap();
}