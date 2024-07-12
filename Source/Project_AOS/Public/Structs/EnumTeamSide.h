// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETeamSideBase : uint8
{
	Type	= 0x00		UMETA(Hidden),
	Blue	= 0x01 << 0 UMETA(DisplayName = "Blue"),
	Red		= 0x01 << 1 UMETA(DisplayName = "Red"),
	Neutral = 0x01 << 1 UMETA(DisplayName = "Neutral"),
};
ENUM_CLASS_FLAGS(ETeamSideBase);

// 플레이어 상태를 나타내는 enum 클래스
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EPlayerState : uint8
{
	Type	= 0x00		UMETA(Hidden),
	Alive	= 0x01 << 0 UMETA(DisplayName = "Alive"),
	Dead	= 0x01 << 1 UMETA(DisplayName = "Dead"),
	Exiting = 0x01 << 1 UMETA(DisplayName = "Exiting"),
};
ENUM_CLASS_FLAGS(EPlayerState);

/**
 * 
 */
class PROJECT_AOS_API EnumTeamSide
{
public:
	EnumTeamSide();
	~EnumTeamSide();
};
