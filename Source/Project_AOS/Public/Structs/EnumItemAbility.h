// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EItemAbility : uint32
{
	None,
	MaxHealthPoints,
	MaxManaPoints,
	HealthRegeneration,
	ManaRegeneration,
	AttackDamage,
	AbilityPower,
	DefensePower,
	MagicResistance,
	AttackSpeed,
	AbilityHaste,
	CriticalChance,
	MovementSpeed,
};
ENUM_CLASS_FLAGS(EItemAbility);

/**
 * 
 */
class PROJECT_AOS_API EnumItemAbility
{
public:
	EnumItemAbility();
	~EnumItemAbility();
};
