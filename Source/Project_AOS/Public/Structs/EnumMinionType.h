// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMinionType : uint8
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
