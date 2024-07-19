// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBaseCrowdControl : uint32
{
	None			= 0x00,
	Slow			= 0x01 << 0,	// 이동속도 둔화
	Cripple			= 0x01 << 1,	// 공격속도 둔화
	Silence			= 0x01 << 2,	// 침묵
	Blind			= 0x01 << 3,	// 실명
	BlockedSight	= 0x01 << 4,	// 시야 차단
	Snare			= 0x01 << 5,	// 속박
	Stun			= 0x01 << 6,	// 기절
	Taunt			= 0x01 << 7,	// 도발
};
ENUM_CLASS_FLAGS(EBaseCrowdControl);

/**
 * 
 */
class PROJECT_AOS_API EnumCrowdControl
{
public:
	EnumCrowdControl();
	~EnumCrowdControl();
};
