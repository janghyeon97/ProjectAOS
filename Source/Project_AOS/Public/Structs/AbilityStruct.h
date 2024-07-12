// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AbilityStruct.generated.h"

USTRUCT(BlueprintType)
struct FAbilityInfomation
{
	GENERATED_BODY()

public:
	FAbilityInfomation()
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
};

USTRUCT(BlueprintType)
struct FAbilityStat
{
	GENERATED_BODY()

public:
	FAbilityStat()
	{
	};

	bool operator==(const FAbilityStat& Other) const
	{
		return Name == Other.Name;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Ability_AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Ability_AbilityPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Ability_AD_Ratio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Ability_AP_Ratio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReuseDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> UniqueAttributes;
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
	FAbilityInfomation AbilityInfomation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAbilityStat> AbilityStatInfomation;
};

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UAbilityStruct : public UObject
{
	GENERATED_BODY()
	
};
