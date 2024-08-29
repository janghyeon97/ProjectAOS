// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AOSGameInstance.h"
#include "DataProviders/ChampionDataProvider.h" 
#include "DataProviders/MinionDataProvider.h" 

UAOSGameInstance::UAOSGameInstance()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DT_CHAMPIONSLIST(TEXT("/Game/ProjectAOS/DataTables/DT_ChampionsList.DT_ChampionsList"));
	if (DT_CHAMPIONSLIST.Succeeded()) ChampionsList = DT_CHAMPIONSLIST.Object;

	static ConstructorHelpers::FObjectFinder<UDataTable> MINION_DATATABLE(TEXT("/Game/ProjectAOS/DataTables/Minions/DT_MinionDataTable.DT_MinionDataTable"));
	if (MINION_DATATABLE.Succeeded()) MinionDataTable = MINION_DATATABLE.Object;

	static ConstructorHelpers::FObjectFinder<UDataTable> GAME_DATATABLE(TEXT("/Game/ProjectAOS/DataTables/DT_GameDataTable.DT_GameDataTable"));
	if (GAME_DATATABLE.Succeeded()) GameDataTable = GAME_DATATABLE.Object;

	static ConstructorHelpers::FObjectFinder<UDataTable> SHAREDGAMEPLAY_DATATABLE(TEXT("/Game/ProjectAOS/DataTables/DT_SharedGamePlayParticles.DT_SharedGamePlayParticles"));
	if (SHAREDGAMEPLAY_DATATABLE.Succeeded()) SharedGamePlayParticlesDataTable = SHAREDGAMEPLAY_DATATABLE.Object;

	static ConstructorHelpers::FClassFinder<UChampionDataProvider> ChampionProviderBPClass(TEXT("/Script/Project_AOS.ChampionDataProvider"));
	if (ChampionProviderBPClass.Succeeded()) DataProviderClasses.Add(EObjectType::Player, ChampionProviderBPClass.Class);

	static ConstructorHelpers::FClassFinder<UMinionDataProvider> MinionProviderBPClass(TEXT("/Script/Project_AOS.MinionDataProvider"));
	if (MinionProviderBPClass.Succeeded()) DataProviderClasses.Add(EObjectType::Minion, MinionProviderBPClass.Class);
	
	NumberOfPlayer = -1;
}

void UAOSGameInstance::Init()
{
	Super::Init();

	if (!::IsValid(ChampionsList))
	{
		UE_LOG(LogTemp, Error, TEXT("ChampionsList is not valid."));
		return;
	}

	if (!::IsValid(MinionDataTable))
	{
		UE_LOG(LogTemp, Error, TEXT("MinionDataTable is not valid."));
		return;
	}

	InitializeProvider(EObjectType::Player, ChampionsList);
	InitializeProvider(EObjectType::Minion, MinionDataTable);
}

void UAOSGameInstance::InitializeProvider(EObjectType ObjectType, UDataTable* DataTable)
{
	if (!DataProviderClasses.Contains(ObjectType))
	{
		UE_LOG(LogTemp, Error, TEXT("DataProviderClasses does not contain key for ObjectType: %d"), ObjectType);
		return;
	}

	UClass* ProviderClass = *DataProviderClasses.Find(ObjectType);
	if (ProviderClass)
	{
		UCharacterDataProviderBase* Provider = NewObject<UCharacterDataProviderBase>(this, ProviderClass);
		if (Provider)
		{
			Provider->Init(DataTable);
			DataProviders.Add(ObjectType, Provider);
			UE_LOG(LogTemp, Log, TEXT("Provider for ObjectType: %d initialized and added to DataProviders."), ObjectType);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Provider for ObjectType: %d."), ObjectType);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ProviderClass for ObjectType: %d is not valid."), ObjectType);
	}
}


void UAOSGameInstance::Shutdown()
{
	Super::Shutdown();
}

const UDataTable* UAOSGameInstance::GetCampionsListTable() const 
{
	return ChampionsList;
}

const FChampionsListRow* UAOSGameInstance::GetCampionsListTableRow(const FName& RowName) const
{
	if (!::IsValid(ChampionsList))
	{
		UE_LOG(LogTemp, Error, TEXT("ChampionsList is not valid."));
		return nullptr;
	}

	if (RowName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("RowName is empty."));
		return nullptr;
	}

	const FChampionsListRow* ChampionsListRow = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""));
	if (!ChampionsListRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find row in ChampionsList for RowName: %s"), *RowName.ToString());
		return nullptr;
	}

	return ChampionsListRow;
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

const UDataTable* UAOSGameInstance::GetCharacterResourcesTable(const FName& RowName)
{
	if (::IsValid(ChampionsList) == false)
	{
		return nullptr;
	}

	UDataTable* DataTable = ChampionsList->FindRow<FChampionsListRow>(RowName, TEXT(""))->CharacterResourcesTable;
	if (::IsValid(DataTable) == false)
	{
		return nullptr;
	}

	return DataTable;
}

const TArray<FCharacterAnimationAttribute>& UAOSGameInstance::GetCharacterAnimMontages(const FName& RowName)
{
	const UDataTable* DataTable = GetCharacterResourcesTable(RowName);
	if (!::IsValid(DataTable))
	{
		return EmptyAnimationArray;
	}

	FCharacterGamePlayDataRow* DataRow = DataTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (!DataRow)
	{
		UE_LOG(LogTemp, Error, TEXT("GetCharacterAnimMontages: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
		return EmptyAnimationArray;
	}

	return DataRow->GameplayMontages;
}

const TArray<FCharacterParticleEffectAttribute>& UAOSGameInstance::GetCharacterParticleEffects(const FName& RowName)
{
	const UDataTable* DataTable = GetCharacterResourcesTable(RowName);
	if (!::IsValid(DataTable))
	{
		return EmptyParticleArray;
	}

	FCharacterGamePlayDataRow* DataRow = DataTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (!DataRow)
	{
		UE_LOG(LogTemp, Error, TEXT("GetCharacterParticleEffects: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
		return EmptyParticleArray;
	}

	return DataRow->GameplayParticles;
}

const TArray<FCharacterStaticMeshAttribute>& UAOSGameInstance::GetCharacterStaticMeshes(const FName& RowName)
{
	const UDataTable* DataTable = GetCharacterResourcesTable(RowName);
	if (!::IsValid(DataTable))
	{
		return EmptyMeshArray;
	}

	FCharacterGamePlayDataRow* DataRow = DataTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (!DataRow)
	{
		UE_LOG(LogTemp, Error, TEXT("GetCharacterStaticMeshes: Failed to find row in DataTable for ChampionIndex %s."), *RowName.ToString());
		return EmptyMeshArray;
	}

	return DataRow->GameplayMeshes;
}


const UDataTable* UAOSGameInstance::GetMinionDataTable()
{
	return MinionDataTable;
}

const UDataTable* UAOSGameInstance::GetGameDataTable()
{
	return GameDataTable;
}

const UDataTable* UAOSGameInstance::GetSharedGamePlayParticlesDataTable()
{
	return SharedGamePlayParticlesDataTable;
}

FSharedGameplay* UAOSGameInstance::GetSharedGamePlayParticles()
{
	if (!SharedGamePlayParticlesDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("GetSharedGamePlayParticles: SharedGamePlayParticlesDataTable is null."));
		return nullptr;
	}

	FSharedGameplay* SharedGameplayRow = SharedGamePlayParticlesDataTable->FindRow<FSharedGameplay>(FName(*FString::FromInt(1)), TEXT(""));
	if (!SharedGameplayRow)
	{
		UE_LOG(LogTemp, Error, TEXT("GetSharedGamePlayParticles: Failed to find row in SharedGamePlayParticlesDataTable."));
		return nullptr;
	}

	return SharedGameplayRow;
}


UCharacterDataProviderBase* UAOSGameInstance::GetDataProvider(EObjectType ObjectType) const
{
	if (!DataProviders.Contains(ObjectType))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UAOSGameInstance::GetDataProvider] DataProvider for ObjectType %d does not exist"), ObjectType);
		return nullptr;
	}

	return DataProviders[ObjectType];
}