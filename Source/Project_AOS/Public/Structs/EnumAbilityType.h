// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAbilityType : uint32
{
    None        = 0      UMETA(DisplayName = "None"),
    Passive     = 1 << 0 UMETA(DisplayName = "Passive"),
    Active      = 1 << 1 UMETA(DisplayName = "Active"),
    Toggle      = 1 << 2 UMETA(DisplayName = "Toggle"),
    Channel     = 1 << 3 UMETA(DisplayName = "Channel"),
};


UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAbilityComponent : uint32
{
    None                    = 0      UMETA(DisplayName = "None"),
    Scaling                 = 1 << 0 UMETA(DisplayName = "Scaling"),
    Cost                    = 1 << 1 UMETA(DisplayName = "Cost"),
    Cooldown                = 1 << 2 UMETA(DisplayName = "Cooldown"),
    Ammo                    = 1 << 3 UMETA(DisplayName = "Ammo"),
    Summon                  = 1 << 4 UMETA(DisplayName = "Summon"),
    Range                   = 1 << 5 UMETA(DisplayName = "Range"),
    Dash                    = 1 << 6 UMETA(DisplayName = "Dash"),
    Blink                   = 1 << 7 UMETA(DisplayName = "Blink"),
    NormalChannel           = 1 << 8 UMETA(DisplayName = "NormalChannel"),
    MovementChannel         = 1 << 9 UMETA(DisplayName = "MovementChannel"),
    ObjectiveChannel        = 1 << 10 UMETA(DisplayName = "ObjectiveChannel"),
    UninterruptibleChannel  = 1 << 11 UMETA(DisplayName = "UninterruptibleChannel"),
};

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAbilityDetection : uint8
{
    None    = 0      UMETA(DisplayName = "None"),
    Range   = 1 << 0 UMETA(DisplayName = "Range"),
    Dash    = 1 << 1 UMETA(DisplayName = "Dash"),
    Blink   = 1 << 2 UMETA(DisplayName = "Blink"),
};


/**
 * 
 */
class PROJECT_AOS_API EnumAbilityType
{
public:
	EnumAbilityType();
	~EnumAbilityType();
};