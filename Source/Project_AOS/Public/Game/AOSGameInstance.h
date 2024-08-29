// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Structs/EnumAbilityID.h"
#include "Structs/GameData.h"
#include "Structs/AbilityData.h"
#include "Structs/MinionData.h"
#include "Structs/CharacterResources.h"
#include "Structs/EnumCharacterType.h"
#include "DataProviders/CharacterDataProviderBase.h" 
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
	FName ChampionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture* ChampionImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* StatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* AbilityStatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* CharacterResourcesTable;

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
	float AbilityPower;

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

	void InitializeProvider(EObjectType ObjectType, UDataTable* DataTable);

	const UDataTable* GetCampionsListTable() const;
	const FChampionsListRow* GetCampionsListTableRow(const FName& RowName) const;

	const UDataTable* GetCharacterSkinDataTable();
	FCharacterSkinTableRow* GetCharacterSkinDataTableRow(uint32 InLevel);

	const UDataTable* GetCharacterResourcesTable(const FName& RowName);
	const TArray<FCharacterAnimationAttribute>& GetCharacterAnimMontages(const FName& RowName);
	const TArray<FCharacterParticleEffectAttribute>& GetCharacterParticleEffects(const FName& RowName);
	const TArray<FCharacterStaticMeshAttribute>& GetCharacterStaticMeshes(const FName& RowName);

	const UDataTable* GetMinionDataTable();
	const UDataTable* GetGameDataTable();

	const UDataTable* GetSharedGamePlayParticlesDataTable();
	FSharedGameplay* GetSharedGamePlayParticles();

	UCharacterDataProviderBase* GetDataProvider(EObjectType ObjectType) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	uint8 NumberOfPlayer;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* ChampionsList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* CharacterSkinTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* MinionDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* GameDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameInstance", Meta = (AllowPrivateAccess))
	class UDataTable* SharedGamePlayParticlesDataTable;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataProviders", meta = (AllowPrivateAccess = "true"))
	TMap<EObjectType, UClass*> DataProviderClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataProviders", meta = (AllowPrivateAccess = "true"))
	mutable TMap<EObjectType, UCharacterDataProviderBase*> DataProviders;

	// 빈 배열을 미리 정의하여 유효하지 않은 경우 반환할 수 있게 함
	TArray<FCharacterAnimationAttribute> EmptyAnimationArray;
	TArray<FCharacterParticleEffectAttribute> EmptyParticleArray;
	TArray<FCharacterStaticMeshAttribute> EmptyMeshArray;
};
