// Fill out your copyright notice in the Description page of Project Settings.


#include "DataProviders/CharacterDataProviderBase.h"
#include "Structs/CharacterResources.h"
#include "Engine/DataTable.h"

void UCharacterDataProviderBase::Init(UDataTable* InDataTable)
{
	UE_LOG(LogTemp, Error, TEXT("Init is not implemented in %s"), *GetName());
}

const UDataTable* UCharacterDataProviderBase::GetDataTable() const
{
	UE_LOG(LogTemp, Error, TEXT("GetDataTable is not implemented in %s"), *GetName());
	return nullptr;
}

const UDataTable* UCharacterDataProviderBase::GetStatDataTable(const FName& RowName) const
{
	UE_LOG(LogTemp, Error, TEXT("GetStatDataTable is not implemented in %s"), *GetName());
	return nullptr;
}

const FStatTableRow* UCharacterDataProviderBase::GetCharacterStat(const FName& RowName, int32 InLevel) const
{
	UE_LOG(LogTemp, Error, TEXT("GetCharacterStat is not implemented in %s"), *GetName());
	return nullptr;
}

const UDataTable* UCharacterDataProviderBase::GetAbilityStatDataTable(const FName& RowName) const
{
	UE_LOG(LogTemp, Error, TEXT("GetAbilityStatDataTable is not implemented in %s"), *GetName());
	return nullptr;
}

const FAbility* UCharacterDataProviderBase::GetCharacterAbility(const FName& RowName, EAbilityID AbilityID, int32 Level) const
{
	UE_LOG(LogTemp, Error, TEXT("GetCharacterAbility is not implemented in %s"), *GetName());
	return nullptr;
}

const FAbilityInformation* UCharacterDataProviderBase::GetAbilityInfo(const FName& RowName, EAbilityID AbilityID, int32 Level) const
{
	UE_LOG(LogTemp, Error, TEXT("GetAbilityInfo is not implemented in %s"), *GetName());
	return nullptr;
}

const FAbilityStatTable* UCharacterDataProviderBase::GetAbilityStatData(const FName& RowName, EAbilityID AbilityID, int32 Level, int32 InstanceIndex) const
{
	UE_LOG(LogTemp, Error, TEXT("GetAbilityStatData is not implemented in %s"), *GetName());
	return nullptr;
}

const UDataTable* UCharacterDataProviderBase::GetCharacterResourcesTable(const FName& RowName) const
{
	UE_LOG(LogTemp, Error, TEXT("GetCharacterResourcesTable is not implemented in %s"), *GetName());
	return nullptr;
}

const TArray<FCharacterAnimationAttribute>& UCharacterDataProviderBase::GetCharacterAnimMontages(const FName& RowName) const
{
	static TArray<FCharacterAnimationAttribute> Dummy;
	UE_LOG(LogTemp, Error, TEXT("GetCharacterAnimMontages is not implemented in %s"), *GetName());
	return Dummy;
}

const TArray<FCharacterParticleEffectAttribute>& UCharacterDataProviderBase::GetCharacterParticleEffects(const FName& RowName) const
{
	static TArray<FCharacterParticleEffectAttribute> Dummy;
	UE_LOG(LogTemp, Error, TEXT("GetCharacterParticleEffects is not implemented in %s"), *GetName());
	return Dummy;
}

const TArray<FCharacterStaticMeshAttribute>& UCharacterDataProviderBase::GetCharacterStaticMeshes(const FName& RowName) const
{
	static TArray<FCharacterStaticMeshAttribute> Dummy;
	UE_LOG(LogTemp, Error, TEXT("UCharacterDataProviderBase is not implemented in %s"), *GetName());
	return Dummy;
}

const TMap<FName, UAnimMontage*> UCharacterDataProviderBase::GetCharacterMontagesMap(const FName& RowName) const
{
	UE_LOG(LogTemp, Error, TEXT("GetGamePlayMontagesMap is not implemented in %s"), *GetName());
	return TMap<FName, UAnimMontage*>();
}

const TMap<FName, UParticleSystem*> UCharacterDataProviderBase::GetCharacterParticlesMap(const FName& RowName) const
{
	UE_LOG(LogTemp, Error, TEXT("GetGamePlayParticlesMap is not implemented in %s"), *GetName());
	return TMap<FName, UParticleSystem*>();
}

const TMap<FName, UStaticMesh*> UCharacterDataProviderBase::GetCharacterMeshesMap(const FName& RowName) const
{
	UE_LOG(LogTemp, Error, TEXT("GetGamePlayMeshesMap is not implemented in %s"), *GetName());
	return TMap<FName, UStaticMesh*>();
}
