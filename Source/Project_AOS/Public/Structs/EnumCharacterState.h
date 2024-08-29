// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBaseCharacterState : uint32
{
	None			= 0		UMETA(Hidden),
	Death			= 1 << 0 UMETA(DisplayName = "Death"),
	Move			= 1 << 1 UMETA(DisplayName = "Move"),
	Jump			= 1 << 2 UMETA(DisplayName = "Jump"),
	Attacking		= 1 << 3 UMETA(DisplayName = "Attacking"),
	AttackEnded		= 1 << 4 UMETA(DisplayName = "AttackEnded"),
	Recall			= 1 << 5 UMETA(DisplayName = "Recall"),

	Ability_Q		= 1 << 10 UMETA(DisplayName = "Ability_Q"),
	Ability_E		= 1 << 11 UMETA(DisplayName = "Ability_E"),
	Ability_R		= 1 << 12 UMETA(DisplayName = "Ability_R"),
	Ability_LMB		= 1 << 13 UMETA(DisplayName = "Ability_LMB"),
	Ability_RMB		= 1 << 14 UMETA(DisplayName = "Ability_RMB"),
	AbilityUsed		= 1 << 15 UMETA(DisplayName = "AbilityUsed"),
	SwitchAction	= 1 << 16 UMETA(DisplayName = "SwitchAction")
};
ENUM_CLASS_FLAGS(EBaseCharacterState);


/**
 * 
 */
class PROJECT_AOS_API EnumCharacterState
{
public:
	EnumCharacterState();
	~EnumCharacterState();
};
