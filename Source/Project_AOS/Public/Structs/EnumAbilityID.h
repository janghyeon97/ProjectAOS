// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAbilityID : uint8
{
	None,
	Ability_Q,
	Ability_E,
	Ability_R,
	Ability_LMB,
	Ability_RMB
};


class PROJECT_AOS_API EnumAbilityID
{
public:
	EnumAbilityID();
	~EnumAbilityID();

	TMap<EAbilityID, int> AbilityIDRowMap;
};
