// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Structs/EnumAbilityID.h"
#include "Structs/AbilityStruct.h"
#include "Structs/MinionData.h"
#include "AOSGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FChampionsListRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FChampionsListRow()
	{

	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChampionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture* ChampionImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* StatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* AbilityStatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* SkinTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* CharacterClass;
};

USTRUCT(BlueprintType)
struct FStatTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FStatTableRow()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxEXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRegeneration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaRegeneration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MagicResistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CriticalChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> UniqueAttributes;
};

USTRUCT(BlueprintType)
struct FAbilityStatTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FAbilityStatTableRow()
	{

	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbility Ability_Q;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbility Ability_E;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbility Ability_R;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbility Ability_LMB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbility Ability_RMB;
};

USTRUCT(BlueprintType)
struct FCharacterSkinTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FCharacterSkinTableRow()
	{

	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChampionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkinName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture* SkinImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* Mesh;
};

UCLASS()
class PROJECT_AOS_API UAOSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UAOSGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	const UDataTable* GetCampionsListTable();
	FChampionsListRow* GetCampionsListTableRow(uint32 ChampionIndex);

	const UDataTable* GetCharacterStatDataTable(uint32 CharacterIndex);
	FStatTableRow* GetCharacterStat(uint32 CharacterIndex, uint32 InLevel);

	const UDataTable* GetCharacterAbilityStatDataTable(uint32 CharacterIndex);
	FAbility* GetCharacterAbilityStruct(uint32 CharacterIndex, EAbilityID AbilityID, uint32 InLevel);
	FAbilityStat* GetCharacterAbilityStat(uint32 CharacterIndex, EAbilityID AbilityID, uint32 InLevel, uint8 InstanceIndex);

	const UDataTable* GetCharacterSkinDataTable();
	FCharacterSkinTableRow* GetCharacterSkinDataTableRow(uint32 InLevel);

	const UDataTable* GetMinionDataTable();
	FMinionDataTableRow* GetMinionDataTableRow(EMinionType MinionType);
	FStatTableRow* GetMinionStat(EMinionType MinionType);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	uint8 NumberOfPlayer;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* ChampionsList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* CharacterStatDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* CharacterAbilityStatDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* CharacterSkinTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	class UDataTable* MinionDataTable;
};
