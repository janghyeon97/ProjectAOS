// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAbilityEffect : uint32
{
	None			= 0x00			UMETA(Hidden),
	Stun			= 0x01 << 0		UMETA(DisplayName = "Stun"),
	Slow			= 0x01 << 1		UMETA(DisplayName = "Slow"),
	Burn			= 0x01 << 2		UMETA(DisplayName = "Burn"),
	Lifesteal		= 0x01 << 3		UMETA(DisplayName = "Lifesteal"),
	Cripple			= 0x01 << 4		UMETA(DisplayName = "Cripple"),
	Silence			= 0x01 << 5		UMETA(DisplayName = "Silence"),
	Blind			= 0x01 << 6		UMETA(DisplayName = "Blind"),
	BlockedSight	= 0x01 << 7		UMETA(DisplayName = "BlockedSight"),
	Snare			= 0x01 << 8		UMETA(DisplayName = "Snare"),
	Taunt			= 0x01 << 9		UMETA(DisplayName = "Taunt"),
};

/**
 * 
 */
class PROJECT_AOS_API EnumAbilityEffect
{
public:
	EnumAbilityEffect();
	~EnumAbilityEffect();
};
