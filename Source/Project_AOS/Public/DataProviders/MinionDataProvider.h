// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/CharacterDataProviderBase.h"
#include "MinionDataProvider.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UMinionDataProvider : public UCharacterDataProviderBase
{
	GENERATED_BODY()
	
public:
	virtual void Init(UDataTable* InDataTable) override;

	virtual const UDataTable* GetDataTable() const override;

	virtual const UDataTable* GetStatDataTable(const FName& RowName) const override;
	virtual const FStatTableRow* GetCharacterStat(const FName& RowName, int32 InLevel) const override;

	virtual const UDataTable* GetAbilityStatDataTable(const FName& RowName) const override;
	virtual const FAbility* GetCharacterAbility(const FName& RowName, EAbilityID AbilityID, int32 Level) const override;
	virtual const FAbilityInformation* GetAbilityInfo(const FName& RowName, EAbilityID AbilityID, int32 Level) const override;
	virtual const FAbilityStatTable* GetAbilityStatData(const FName& RowName, EAbilityID AbilityID, int32 Level, int32 InstanceIndex) const override;

	virtual const UDataTable* GetCharacterResourcesTable(const FName& RowName) const override;
	virtual const TArray<FCharacterAnimationAttribute>& GetCharacterAnimMontages(const FName& RowName) const override;
	virtual const TArray<FCharacterParticleEffectAttribute>& GetCharacterParticleEffects(const FName& RowName) const override;
	virtual const TArray<FCharacterStaticMeshAttribute>& GetCharacterStaticMeshes(const FName& RowName) const override;

	virtual const TMap<FName, UAnimMontage*> GetCharacterMontagesMap(const FName& RowName) const override;
	virtual const TMap<FName, UParticleSystem*> GetCharacterParticlesMap(const FName& RowName) const override;
	virtual const TMap<FName, UStaticMesh*> GetCharacterMeshesMap(const FName& RowName) const override;

private:
	UPROPERTY(EditAnywhere, Category = "DataTables")
	UDataTable* MinionDataTable;
};
