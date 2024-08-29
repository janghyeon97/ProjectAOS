// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Structs/EnumAbilityID.h"
#include "CharacterDataProviderInterface.generated.h"

struct FStatTableRow;
struct FAbility;
struct FAbilityInformation;
struct FAbilityStatTable;
struct FCharacterAnimationAttribute;
struct FCharacterParticleEffectAttribute;
struct FCharacterStaticMeshAttribute;

UINTERFACE(MinimalAPI)
class UCharacterDataProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class PROJECT_AOS_API ICharacterDataProviderInterface
{
	GENERATED_BODY()

public:
	virtual void Init(UDataTable* InDataTable) = 0;

	virtual const UDataTable* GetDataTable() const = 0;
	
	virtual const UDataTable* GetStatDataTable(const FName& RowName) const = 0;
	virtual const FStatTableRow* GetCharacterStat(const FName& RowName, int32 InLevel) const = 0;

	virtual const UDataTable* GetAbilityStatDataTable(const FName& RowName) const = 0;
	virtual const FAbility* GetCharacterAbility(const FName& RowName, EAbilityID AbilityID, int32 Level) const = 0;
	virtual const FAbilityInformation* GetAbilityInfo(const FName& RowName, EAbilityID AbilityID, int32 Level) const = 0;
	virtual const FAbilityStatTable* GetAbilityStatData(const FName& RowName, EAbilityID AbilityID, int32 Level, int32 InstanceIndex) const = 0;

	virtual const UDataTable* GetCharacterResourcesTable(const FName& RowName) const = 0;
	virtual const TArray<FCharacterAnimationAttribute>& GetCharacterAnimMontages(const FName& RowName) const = 0;
	virtual const TArray<FCharacterParticleEffectAttribute>& GetCharacterParticleEffects(const FName& RowName) const = 0;
	virtual const TArray<FCharacterStaticMeshAttribute>& GetCharacterStaticMeshes(const FName& RowName) const = 0;

	virtual const TMap<FName, UAnimMontage*> GetCharacterMontagesMap(const FName& RowName) const = 0;
	virtual const TMap<FName, UParticleSystem*> GetCharacterParticlesMap(const FName& RowName) const = 0;
	virtual const TMap<FName, UStaticMesh*> GetCharacterMeshesMap(const FName& RowName) const = 0;
};
