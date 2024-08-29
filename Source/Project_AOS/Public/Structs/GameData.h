// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameData.generated.h"


USTRUCT(BlueprintType)
struct FGameDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FGameDataTableRow()
		: IncrementCurrencyAmount(1)
		, MaxAssistTime(8.0f)
		, ExperienceShareRadius(1600.f)
		, ExpShareFactorTwoPlayers(1.6236f)
		, ExpShareFactorThreePlayers(1.4157f)
		, ExpShareFactorFourPlayers(1.2494f)
		, ExpShareFactorFivePlayers(1.0f)
		, MinionsPerWave(6)
		, SuperMinionSpawnInterval(3)
		, SpawnInterval(0.5f)
		, MinionSpawnTime(5.0f)
		, MinionSpawnInterval(90.f)
		, MaxChaseDistance(1000.f)
	{

	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 IncrementCurrencyAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	float MaxAssistTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExperienceShareRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorTwoPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorThreePlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorFourPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorFivePlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion")
	int32 MinionsPerWave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion")
	int32 SuperMinionSpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion")
	float SpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion")
	float MinionSpawnTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion")
	float MinionSpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion")
	float MaxChaseDistance;
};

USTRUCT(BlueprintType)
struct FGameplayParticleAttribute
{
	GENERATED_BODY()

public:
	FGameplayParticleAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FGameplayParticleAttribute(FName InKey, float InValue)
		: Key(InKey)
		, Value(nullptr)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* Value;
};

USTRUCT(BlueprintType)
struct FSharedGameplay : public FTableRowBase
{
	GENERATED_BODY()

public:
	FSharedGameplay()
		: SharedGameplayParticles(TArray<FGameplayParticleAttribute>())
	{

	}

	UParticleSystem* GetSharedGamePlayParticle(const FName& InKey) const
	{
		for (const FGameplayParticleAttribute& Attribute : SharedGameplayParticles)
		{
			if (Attribute.Key.IsEqual(InKey))
			{
				return Attribute.Value;
			}
		}

		return nullptr;
	}

	TMap<FName, UParticleSystem*> GetSharedGamePlayParticlesMap() const
	{
		TMap<FName, UParticleSystem*> AttributesMap;
		for (const FGameplayParticleAttribute& Attribute : SharedGameplayParticles)
		{
			if (Attribute.Key.IsNone() == false)
			{
				AttributesMap.Add(Attribute.Key, Attribute.Value);
			}
		}
		return AttributesMap;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	TArray<FGameplayParticleAttribute> SharedGameplayParticles;
};

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UGameData : public UObject
{
	GENERATED_BODY()
	
};
