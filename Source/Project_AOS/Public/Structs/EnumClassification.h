// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EClassification : uint32
{
	None,
	Starter,
	Potions,
	Consumables,
	Trinkets,
	Boots,
	Basic,
	Epic,
	Legendary,
	Distributed,
	ChampionExclusive,
};
ENUM_CLASS_FLAGS(EClassification);

/**
 * 
 */
class PROJECT_AOS_API EnumClassification
{
public:
	EnumClassification();
	~EnumClassification();
};
