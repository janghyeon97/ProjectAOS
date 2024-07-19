// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/EnumItemAbility.h"
#include "ItemAbilityData.generated.h"

USTRUCT(BlueprintType)
struct FItemAbility
{
	GENERATED_BODY()

public:
	FString AbilityTypeToString() const
	{
		switch (AbilityType)
		{
		case EItemAbility::None:                return TEXT("None");
		case EItemAbility::MaxHealthPoints:     return TEXT("Max Health");
		case EItemAbility::MaxManaPoints:       return TEXT("Max Mana");
		case EItemAbility::HealthRegeneration:  return TEXT("Health Regeneration");
		case EItemAbility::ManaRegeneration:    return TEXT("Mana Regeneration");
		case EItemAbility::AttackDamage:        return TEXT("Attack Damage");
		case EItemAbility::AbilityPower:        return TEXT("Ability Power");
		case EItemAbility::DefensePower:        return TEXT("Defense Power");
		case EItemAbility::MagicResistance:     return TEXT("Magic Resistance");
		case EItemAbility::AttackSpeed:         return TEXT("Attack Speed");
		case EItemAbility::AbilityHaste:        return TEXT("Ability Haste");
		case EItemAbility::CriticalChance:      return TEXT("Critical Chance");
		case EItemAbility::MovementSpeed:       return TEXT("Movement Speed");
		default:                                return TEXT("Unknown Ability");
		}
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	EItemAbility AbilityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	int32 AbilityValue;
};

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UItemAbilityData : public UObject
{
	GENERATED_BODY()
	
};
