// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AOSGameInstance.h"

UAOSGameInstance::UAOSGameInstance()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DT_CHAMPIONSLIST(TEXT("/Game/ProjectAOS/DataTables/DT_ChampionsList.DT_ChampionsList"));
	if (DT_CHAMPIONSLIST.Succeeded()) ChampionsList = DT_CHAMPIONSLIST.Object;

	static ConstructorHelpers::FObjectFinder<UDataTable> MINION_DATATABLE(TEXT("/Game/ProjectAOS/DataTables/Minions/DT_MinionDataTable.DT_MinionDataTable"));
	if (MINION_DATATABLE.Succeeded()) MinionDataTable = MINION_DATATABLE.Object;

	NumberOfPlayer = -1;
}

void UAOSGameInstance::Init()
{
	Super::Init();

	if (::IsValid(ChampionsList) == false || ChampionsList->GetRowMap().Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Not enuough data in ChampionsListTable."));
	}
	else
	{
		for (int32 i = 1; i <= ChampionsList->GetRowMap().Num(); ++i)
		{
			check(nullptr != GetCampionsListTableRow(i))
			check(nullptr != GetCharacterStatDataTable(i))
			check(nullptr != GetCharacterAbilityStatDataTable(i))
		}
	}
}

void UAOSGameInstance::Shutdown()
{
	Super::Shutdown();
}

const UDataTable* UAOSGameInstance::GetCampionsListTable()
{
	return ChampionsList;
}

FChampionsListRow* UAOSGameInstance::GetCampionsListTableRow(uint32 ChampionIndex)
{
	if (::IsValid(ChampionsList))
	{
		return ChampionsList->FindRow<FChampionsListRow>(FName(*FString::FromInt(ChampionIndex)), TEXT(""));
	}
	return nullptr;
}

const UDataTable* UAOSGameInstance::GetCharacterStatDataTable(uint32 ChampionIndex)
{
	if (::IsValid(ChampionsList) == false)
	{
		return nullptr;
	}

	UDataTable* DataTable = ChampionsList->FindRow<FChampionsListRow>(FName(*FString::FromInt(ChampionIndex)), TEXT(""))->StatTable;
	if (::IsValid(DataTable) == false)
	{
		return nullptr;
	}
	return DataTable;
}

FStatTableRow* UAOSGameInstance::GetCharacterStat(uint32 ChampionIndex, uint32 InLevel)
{
	if (::IsValid(ChampionsList) == false)
	{
		return nullptr;
	}

	UDataTable* DataTable = ChampionsList->FindRow<FChampionsListRow>(FName(*FString::FromInt(ChampionIndex)), TEXT(""))->StatTable;
	if (::IsValid(DataTable) == false)
	{
		return nullptr;
	}

	return DataTable->FindRow<FStatTableRow>(FName(*FString::FromInt(InLevel)), TEXT(""));
}

const UDataTable* UAOSGameInstance::GetCharacterAbilityStatDataTable(uint32 ChampionIndex)
{
	if (::IsValid(ChampionsList) == false)
	{
		return nullptr;
	}

	const UDataTable* DataTable = ChampionsList->FindRow<FChampionsListRow>(FName(*FString::FromInt(ChampionIndex)), TEXT(""))->AbilityStatTable;
	if (::IsValid(DataTable) == false)
	{
		return nullptr;
	}

	return DataTable;
}

FAbility* UAOSGameInstance::GetCharacterAbilityStruct(uint32 ChampionIndex, EAbilityID AbilityID, uint32 InLevel)
{
	if (::IsValid(ChampionsList) == false)
	{
		return nullptr;
	}

	UDataTable* AbilityStatTable = ChampionsList->FindRow<FChampionsListRow>(FName(*FString::FromInt(ChampionIndex)), TEXT(""))->AbilityStatTable;
	if (::IsValid(AbilityStatTable) == false)
	{
		return nullptr;
	}

	uint8 MaxLevel = 0;
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		MaxLevel = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""))->Ability_Q.AbilityInfomation.MaxLevel;
		break;
	case EAbilityID::Ability_E:
		MaxLevel = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""))->Ability_E.AbilityInfomation.MaxLevel;
		break;
	case EAbilityID::Ability_R:
		MaxLevel = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""))->Ability_R.AbilityInfomation.MaxLevel;
		break;
	case EAbilityID::Ability_LMB:
		MaxLevel = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""))->Ability_LMB.AbilityInfomation.MaxLevel;
		break;
	case EAbilityID::Ability_RMB:
		MaxLevel = AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(1)), TEXT(""))->Ability_RMB.AbilityInfomation.MaxLevel;
		break;
	}

	uint8 Level = FMath::Clamp<uint8>(InLevel, 1, MaxLevel);

	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		return &AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(InLevel)), TEXT(""))->Ability_Q;
		break;
	case EAbilityID::Ability_E:
		return &AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(InLevel)), TEXT(""))->Ability_E;
		break;
	case EAbilityID::Ability_R:
		return &AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(InLevel)), TEXT(""))->Ability_R;
		break;
	case EAbilityID::Ability_LMB:
		return &AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(InLevel)), TEXT(""))->Ability_LMB;
		break;
	case EAbilityID::Ability_RMB:
		return &AbilityStatTable->FindRow<FAbilityStatTableRow>(FName(*FString::FromInt(InLevel)), TEXT(""))->Ability_RMB;
		break;
	}
	return nullptr;
}

FAbilityStat* UAOSGameInstance::GetCharacterAbilityStat(uint32 ChampionIndex, EAbilityID AbilityID, uint32 InLevel, uint8 InstanceIndex)
{
	FAbility* DataTable = GetCharacterAbilityStruct(ChampionIndex, AbilityID, InLevel);
	if (!DataTable)
	{
		return nullptr;
	}

	uint8 InstanceLevel = FMath::Clamp<uint8>(InstanceIndex, 1, DataTable->AbilityInfomation.MaxInstances);
	return &DataTable->AbilityStatInfomation[InstanceLevel - 1];
}

const UDataTable* UAOSGameInstance::GetCharacterSkinDataTable()
{
	return CharacterSkinTable;
}

FCharacterSkinTableRow* UAOSGameInstance::GetCharacterSkinDataTableRow(uint32 InLevel)
{
	if (::IsValid(CharacterSkinTable) == true)
	{
		return CharacterSkinTable->FindRow<FCharacterSkinTableRow>(FName(*FString::FromInt(InLevel)), TEXT(""));
	}
	return nullptr;
}

const UDataTable* UAOSGameInstance::GetMinionDataTable()
{
	return MinionDataTable;
}

FMinionDataTableRow* UAOSGameInstance::GetMinionDataTableRow(EMinionType MinionType)
{
	if (!::IsValid(MinionDataTable))
	{
		return nullptr;
	}

	for (int32 i = 1; i <= MinionDataTable->GetRowMap().Num(); ++i)
	{
		EMinionType Result = MinionDataTable->FindRow<FMinionDataTableRow>(FName(*FString::FromInt(i)), TEXT(""))->MinionType;
		if (MinionType == Result)
		{
			return MinionDataTable->FindRow<FMinionDataTableRow>(FName(*FString::FromInt(i)), TEXT(""));
		}
	}

	return nullptr;
}

FStatTableRow* UAOSGameInstance::GetMinionStat(EMinionType MinionType)
{
	const UDataTable* DataTable = GetMinionDataTableRow(MinionType)->StatTable;
	if (!DataTable)
	{
		return nullptr;
	}

	FStatTableRow* FoundRow = nullptr;
	switch (MinionType)
	{
	case EMinionType::Melee:
		FoundRow = DataTable->FindRow<FStatTableRow>(FName(*FString::FromInt(1)), TEXT(""));
		break;
	case EMinionType::Ranged:
		FoundRow = DataTable->FindRow<FStatTableRow>(FName(*FString::FromInt(2)), TEXT(""));
		break;
	case EMinionType::Siege:
		FoundRow = DataTable->FindRow<FStatTableRow>(FName(*FString::FromInt(3)), TEXT(""));
		break;
	case EMinionType::Super:
		FoundRow = DataTable->FindRow<FStatTableRow>(FName(*FString::FromInt(4)), TEXT(""));
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid MinionType"));
		return nullptr;
	}

	if (FoundRow == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find row for MinionType: %d"), static_cast<int32>(MinionType));
		return nullptr;
	}

	return FoundRow;
}