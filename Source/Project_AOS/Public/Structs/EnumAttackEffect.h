// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAttackEffect : uint32
{
	None			= 0x00 UMETA(Hidden),
	OnHit			= 0x01 << 0, // 챔피언의 기본 공격이 목표물에 명중하면 효과가 발생합니다.
	OnAttack		= 0x01 << 1, // OnHit의 상위 효과이며 실명 및 평타 판정에 막히지 않습니다.
	AbilityEffects	= 0x01 << 2, // 챔피언의 스킬이 목표물에 명중하면 트리거 됩니다.
};
ENUM_CLASS_FLAGS(EAttackEffect);

/**
 * 
 */
class PROJECT_AOS_API EnumAttackEffect
{
public:
	EnumAttackEffect();
	~EnumAttackEffect();
};
