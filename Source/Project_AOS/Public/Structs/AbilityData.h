// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/EnumAbilityType.h"
#include "AbilityData.generated.h"

USTRUCT(BlueprintType)
struct FAbilityAttribute
{
	GENERATED_BODY()

public:
	FAbilityAttribute()
		: Key(TEXT(""))
		, Value(0.f)
	{
	}

	FAbilityAttribute(FString InKey, float InValue)
		: Key(InKey)
		, Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;
};

USTRUCT(BlueprintType)
struct FAbilityInformation
{
	GENERATED_BODY()

public:
	FAbilityInformation()
	{
	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxInstances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbilityType AbilityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbilityDetection AbilityDetection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EAbilityComponent> Components;
};

USTRUCT(BlueprintType)
struct FAbilityStatTable
{
	GENERATED_BODY()

public:
	FAbilityStatTable()
	{
	};

	bool operator==(const FAbilityStatTable& Other) const
	{
		return Name == Other.Name;
	}

	bool IsValid() const
	{
		return !Name.IsEmpty();
	}

	void AddUniqueAttribute(FString Key, float Value)
	{
		UniqueAttributes.Add(FAbilityAttribute(Key, Value));
	}

	float GetUniqueAttribute(const FString& InKey) const
	{
		for (const FAbilityAttribute& Attribute : UniqueAttributes)
		{
			if (Attribute.Key.Equals(InKey))
			{
				return Attribute.Value;
			}
		}

		return 0.f;
	}

	TMap<FString, float> GetUniqueAttributesMap() const
	{
		TMap<FString, float> AttributesMap;
		for (const FAbilityAttribute& Attribute : UniqueAttributes)
		{
			if (Attribute.Key.IsEmpty() == false)
			{
				AttributesMap.Add(Attribute.Key, Attribute.Value);
			}
		}
		return AttributesMap;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AbilityDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AD_PowerScaling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AP_PowerScaling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReuseDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAbilityAttribute> UniqueAttributes;
};

USTRUCT(BlueprintType)
struct FAbility
{
	GENERATED_BODY()

public:
	FAbility()
	{

	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityInformation AbilityInformation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAbilityStatTable> AbilityStatInformation;
};



/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UAbilityData : public UObject
{
	GENERATED_BODY()
	
};
