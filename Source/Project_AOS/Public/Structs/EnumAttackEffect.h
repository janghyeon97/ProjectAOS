// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAttackEffect : uint32
{
	None			= 0x00 UMETA(Hidden),
	OnHit			= 0x01 << 0, // è�Ǿ��� �⺻ ������ ��ǥ���� �����ϸ� ȿ���� �߻��մϴ�.
	OnAttack		= 0x01 << 1, // OnHit�� ���� ȿ���̸� �Ǹ� �� ��Ÿ ������ ������ �ʽ��ϴ�.
	AbilityEffects	= 0x01 << 2, // è�Ǿ��� ��ų�� ��ǥ���� �����ϸ� Ʈ���� �˴ϴ�.
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
