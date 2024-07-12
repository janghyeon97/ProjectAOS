// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMinionType : uint32
{
	None,
	Melee,
	Ranged,
	Siege,
	Super
};
ENUM_CLASS_FLAGS(EMinionType);

/**
 * 
 */
class PROJECT_AOS_API EnumMinionType
{
public:
	EnumMinionType();
	~EnumMinionType();
};
