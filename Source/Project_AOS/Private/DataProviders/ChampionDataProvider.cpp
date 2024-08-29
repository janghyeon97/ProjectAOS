// Fill out your copyright notice in the Description page of Project Settings.


#include "DataProviders/ChampionDataProvider.h"
#include "Game/AOSGameInstance.h"

void UChampionDataProvider::Init(UDataTable* InDataTable)
{
    ChampionsList = InDataTable;
}

const UDataTable* UChampionDataProvider::GetDataTable() const
{
    return ChampionsList;
}

const UDataTable* UChampionDataProvider::GetStatDataTable(const FName& RowName) const
{
    if (::IsValid(ChampionsList) == false)
    {
        return nullptr;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return nullptr;
    }

    const UDataTable* StatTable = ChampionsListRow->StatTable;
    if (::IsValid(StatTable) == false)
    {
        return nullptr;
    }

    return StatTable;
}

const FStatTableRow* UChampionDataProvider::GetCharacterStat(const FName& RowName, int32 InLevel) const
{
    if (::IsValid(ChampionsList) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetCharacterStat] ChampionsList is invalid."));
        return nullptr;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetCharacterStat] Failed to find ChampionsListRow for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    const UDataTable* StatTable = ChampionsListRow->StatTable;
    if (::IsValid(StatTable) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetCharacterStat] StatTable is invalid for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    uint8 ClampedLevel = FMath::Clamp<uint8>(InLevel, 1, 18);

    FStatTableRow* StatRow = StatTable->FindRow<FStatTableRow>(FName(*FString::FromInt(ClampedLevel)), TEXT(""));
    if (!StatRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetCharacterStat] Failed to find StatRow for Level: %d in StatTable for RowName: %s"), ClampedLevel, *RowName.ToString());
        return nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("[UChampionDataProvider::GetCharacterStat] Successfully found StatRow for RowName: %s at Level: %d"), *RowName.ToString(), ClampedLevel);
    return StatRow;
}


const UDataTable* UChampionDataProvider::GetAbilityStatDataTable(const FName& RowName) const
{
    if (::IsValid(ChampionsList) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetAbilityStatDataTable] ChampionsList is invalid."));
        return nullptr;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetAbilityStatDataTable] Failed to find ChampionsListRow for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    const UDataTable* AbilityStatTable = ChampionsListRow->AbilityStatTable;
    if (::IsValid(AbilityStatTable) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetAbilityStatDataTable] AbilityStatTable is invalid for RowName: %s"), *RowName.ToString());
        return nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("[UChampionDataProvider::GetAbilityStatDataTable] Successfully found AbilityStatTable for RowName: %s"), *RowName.ToString());
    return AbilityStatTable;
}


const FAbility* UChampionDataProvider::GetCharacterAbility(const FName& RowName, EAbilityID AbilityID, int32 Level) const
{
    if (!::IsValid(ChampionsList))
    {
        return nullptr;
    }

    // ChampionDataTable에서 RowName에 해당하는 행을 찾습니다.
    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return nullptr;
    }

    UDataTable* AbilityStatTable = ChampionsListRow->AbilityStatTable;
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


const FAbilityInformation* UChampionDataProvider::GetAbilityInfo(const FName& RowName, EAbilityID AbilityID, int32 Level) const
{
    if (!::IsValid(ChampionsList))
    {
        return nullptr;
    }

    // ChampionDataTable에서 RowName에 해당하는 행을 찾습니다.
    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return nullptr;
    }

    UDataTable* AbilityStatTable = ChampionsListRow->AbilityStatTable;
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


const FAbilityStatTable* UChampionDataProvider::GetAbilityStatData(const FName& RowName, EAbilityID AbilityID, int32 Level, int32 InstanceIndex) const
{
    if (!::IsValid(ChampionsList))
    {
        return nullptr;
    }

    // ChampionDataTable에서 RowName에 해당하는 행을 찾습니다.
    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return nullptr;
    }

    UDataTable* AbilityStatTable = ChampionsListRow->AbilityStatTable;
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
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetAbilityStatData] Failed to find StatRow for Level: 1 in AbilityStatTable for RowName: %s"), *RowName.ToString());
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
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetAbilityStatData] Failed to find StatRow for Level: %d in AbilityStatTable for RowName: %s"), ClampedLevel, *RowName.ToString());
        return nullptr;
    }

    const FAbility* Ability = GetAbility(ClampedStatRow);
    if (!Ability)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetAbilityStatData] Failed to get Ability for RowName: %s at Level: %d"), *RowName.ToString(), ClampedLevel);
        return nullptr;
    }

    int32 InstanceLevel = FMath::Clamp<int32>(InstanceIndex, 1, Ability->AbilityInformation.MaxInstances);
    if (Ability->AbilityStatInformation.IsValidIndex(InstanceLevel - 1))
    {
        return &Ability->AbilityStatInformation[InstanceLevel - 1];
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UChampionDataProvider::GetAbilityStatData] Invalid InstanceLevel: %d for RowName: %s at Level: %d"), InstanceLevel, *RowName.ToString(), ClampedLevel);
        return nullptr;
    }
}

const UDataTable* UChampionDataProvider::GetCharacterResourcesTable(const FName& RowName) const
{
    if (::IsValid(ChampionsList) == false)
    {
        return nullptr;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return nullptr;
    }

    UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
    if (::IsValid(ResourcesTable) == false)
    {
        return nullptr;
    }

    return ResourcesTable;
}

const TArray<FCharacterAnimationAttribute>& UChampionDataProvider::GetCharacterAnimMontages(const FName& RowName) const
{
    static const TArray<FCharacterAnimationAttribute> EmptyMontages;

    if (::IsValid(ChampionsList) == false)
    {
        return EmptyMontages;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return EmptyMontages;
    }

    UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
    if (::IsValid(ResourcesTable) == false)
    {
        return EmptyMontages;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("GetCharacterAnimMontages: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
        return EmptyMontages;
    }

    return DataRow->GameplayMontages;
}

const TArray<FCharacterParticleEffectAttribute>& UChampionDataProvider::GetCharacterParticleEffects(const FName& RowName) const
{
    static const TArray<FCharacterParticleEffectAttribute> EmptyParticles;

    if (::IsValid(ChampionsList) == false)
    {
        return EmptyParticles;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return EmptyParticles;
    }

    UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
    if (::IsValid(ResourcesTable) == false)
    {
        return EmptyParticles;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("GetCharacterParticleEffects: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
        return EmptyParticles;
    }

    return DataRow->GameplayParticles;
}

const TArray<FCharacterStaticMeshAttribute>& UChampionDataProvider::GetCharacterStaticMeshes(const FName& RowName) const
{
    static const TArray<FCharacterStaticMeshAttribute> EmptyMeshes;

    if (::IsValid(ChampionsList) == false)
    {
        return EmptyMeshes;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return EmptyMeshes;
    }

    UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
    if (::IsValid(ResourcesTable) == false)
    {
        return EmptyMeshes;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("GetCharacterStaticMeshes: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
        return EmptyMeshes;
    }

    return DataRow->GameplayMeshes;
}

const TMap<FName, UAnimMontage*> UChampionDataProvider::GetCharacterMontagesMap(const FName& RowName) const
{
    static const TMap<FName, UAnimMontage*> EmptyMontagesMap;

    if (::IsValid(ChampionsList) == false)
    {
        return EmptyMontagesMap;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return EmptyMontagesMap;
    }

    UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
    if (::IsValid(ResourcesTable) == false)
    {
        return EmptyMontagesMap;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("GetCharacterMontagesMap: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
        return EmptyMontagesMap;
    }

    return DataRow->GetGamePlayMontagesMap();
}

const TMap<FName, UParticleSystem*> UChampionDataProvider::GetCharacterParticlesMap(const FName& RowName) const
{
    static const TMap<FName, UParticleSystem*> EmptyParticlesMap;

    if (::IsValid(ChampionsList) == false)
    {
        return EmptyParticlesMap;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return EmptyParticlesMap;
    }

    UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
    if (::IsValid(ResourcesTable) == false)
    {
        return EmptyParticlesMap;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("GetCharacterParticlesMap: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
        return EmptyParticlesMap;
    }

    return DataRow->GetGamePlayParticlesMap();
}

const TMap<FName, UStaticMesh*> UChampionDataProvider::GetCharacterMeshesMap(const FName& RowName) const
{
    static const TMap<FName, UStaticMesh*> EmptyMeshesMap;

    if (::IsValid(ChampionsList) == false)
    {
        return EmptyMeshesMap;
    }

    const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
    if (!ChampionsListRow)
    {
        return EmptyMeshesMap;
    }

    UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
    if (::IsValid(ResourcesTable) == false)
    {
        return EmptyMeshesMap;
    }

    const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("GetCharacterMeshesMap: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
        return EmptyMeshesMap;
    }

    return DataRow->GetGamePlayMeshesMap();
}
