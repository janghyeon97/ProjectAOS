// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETeamSideBase : uint8
{
	Type	UMETA(Hidden),
	Blue	UMETA(DisplayName = "Blue"),
	Red		UMETA(DisplayName = "Red"),
	Neutral UMETA(DisplayName = "Neutral"),
};


// 플레이어 상태를 나타내는 enum 클래스
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Type,
	Alive,
	Death,
	Exiting,
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
