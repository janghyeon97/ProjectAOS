// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDamageType : uint8
{
	None		= 0x00		UMETA(Hidden),
	Physical	= 0x01 << 0 UMETA(DisplayName = "Physical"),
	Magic		= 0x01 << 1 UMETA(DisplayName = "Magic"),
	TrueDamage	= 0x01 << 2 UMETA(DisplayName = "TrueDamage"),
	Critical	= 0x01 << 3 UMETA(DisplayName = "Critical"),
};
ENUM_CLASS_FLAGS(EDamageType);

/**
 * 
 */
class PROJECT_AOS_API EnumDamageType
{
public:
	EnumDamageType();
	~EnumDamageType();
};
