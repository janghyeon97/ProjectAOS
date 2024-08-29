// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterResources.generated.h"


USTRUCT(BlueprintType)
struct FCharacterAnimationAttribute
{
	GENERATED_BODY()

public:
	FCharacterAnimationAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterAnimationAttribute(FName InKey, UAnimMontage* InValue)
		: Key(InKey)
		, Value(nullptr)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Value;
};

USTRUCT(BlueprintType)
struct FCharacterParticleEffectAttribute
{
	GENERATED_BODY()

public:
	FCharacterParticleEffectAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterParticleEffectAttribute(FName InKey, UParticleSystem* InValue)
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
struct FCharacterStaticMeshAttribute
{
	GENERATED_BODY()

public:
	FCharacterStaticMeshAttribute()
		: Key(NAME_None)
		, Value(nullptr)
	{
	}

	FCharacterStaticMeshAttribute(FName InKey, UStaticMesh* InValue)
		: Key(InKey)
		, Value(nullptr)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Value;
};

USTRUCT(BlueprintType)
struct FCharacterGamePlayDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FCharacterGamePlayDataRow()
		: GameplayMontages(TArray<FCharacterAnimationAttribute>())
		, GameplayParticles(TArray<FCharacterParticleEffectAttribute>())
		, GameplayMeshes(TArray<FCharacterStaticMeshAttribute>())
	{
	}

	UAnimMontage* GetGamePlayMontage(const FName& InKey) const
	{
		for (const FCharacterAnimationAttribute& Attribute : GameplayMontages)
		{
			if (Attribute.Key.IsEqual(InKey))
			{
				return Attribute.Value;
			}
		}

		return nullptr;
	}

	UParticleSystem* GetGamePlayParticle(const FName& InKey) const
	{
		for (const FCharacterParticleEffectAttribute& Attribute : GameplayParticles)
		{
			if (Attribute.Key.IsEqual(InKey))
			{
				return Attribute.Value;
			}
		}

		return nullptr;
	}

	UStaticMesh* GetGamePlayMesh(const FName& InKey) const
	{
		for (const FCharacterStaticMeshAttribute& Attribute : GameplayMeshes)
		{
			if (Attribute.Key.IsEqual(InKey))
			{
				return Attribute.Value;
			}
		}

		return nullptr;
	}

	TMap<FName, UAnimMontage*> GetGamePlayMontagesMap() const
	{
		TMap<FName, UAnimMontage*> AttributesMap;
		for (const FCharacterAnimationAttribute& Attribute : GameplayMontages)
		{
			if (!Attribute.Key.IsNone())
			{
				AttributesMap.Add(Attribute.Key, Attribute.Value);
			}
		}
		return AttributesMap;
	}

	TMap<FName, UParticleSystem*> GetGamePlayParticlesMap() const
	{
		TMap<FName, UParticleSystem*> AttributesMap;
		for (const FCharacterParticleEffectAttribute& Attribute : GameplayParticles)
		{
			if (!Attribute.Key.IsNone())
			{
				AttributesMap.Add(Attribute.Key, Attribute.Value);
			}
		}
		return AttributesMap;
	}

	TMap<FName, UStaticMesh*> GetGamePlayMeshesMap() const
	{
		TMap<FName, UStaticMesh*> AttributesMap;
		for (const FCharacterStaticMeshAttribute& Attribute : GameplayMeshes)
		{
			if (!Attribute.Key.IsNone())
			{
				AttributesMap.Add(Attribute.Key, Attribute.Value);
			}
		}
		return AttributesMap;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterAnimationAttribute> GameplayMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterParticleEffectAttribute> GameplayParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<FCharacterStaticMeshAttribute> GameplayMeshes;
};


/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UCharacterResources : public UObject
{
	GENERATED_BODY()
	
};
