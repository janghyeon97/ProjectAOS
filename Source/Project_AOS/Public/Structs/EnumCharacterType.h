// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECharacterType : uint32
{
	None		= 0x00		UMETA(Hidden),
	Player		= 0x01 << 0	UMETA(DisplayName = "Player"),
	Minion		= 0x01 << 1	UMETA(DisplayName = "Minion"),
	Monster		= 0x01 << 2	UMETA(DisplayName = "Monster"),
	AICharacter = 0x01 << 2	UMETA(DisplayName = "AI")
};
ENUM_CLASS_FLAGS(ECharacterType);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EObjectType : uint32
{
	None			= 0x00		UMETA(Hidden),
	Player			= 0x01 << 0	UMETA(DisplayName = "Player"),
	Minion			= 0x01 << 1	UMETA(DisplayName = "Minion"),
	Monster			= 0x01 << 2	UMETA(DisplayName = "Monster"),
	AICharacter		= 0x01 << 3	UMETA(DisplayName = "AI"),
	WorldStatic		= 0x01 << 4	UMETA(DisplayName = "WorldStatic"),
	WorldDynamic	= 0x01 << 5	UMETA(DisplayName = "WorldDynamic")
};
ENUM_CLASS_FLAGS(EObjectType);

/**
 * 
 */
class PROJECT_AOS_API EnumCharacterType
{
public:
	EnumCharacterType();
	~EnumCharacterType();
};
