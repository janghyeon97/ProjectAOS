// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBaseCrowdControl : uint32
{
	None			= 0x00,
	Slow			= 0x01 << 0,	// �̵��ӵ� ��ȭ
	Cripple			= 0x01 << 1,	// ���ݼӵ� ��ȭ
	Silence			= 0x01 << 2,	// ħ��
	Blind			= 0x01 << 3,	// �Ǹ�
	BlockedSight	= 0x01 << 4,	// �þ� ����
	Snare			= 0x01 << 5,	// �ӹ�
	Stun			= 0x01 << 6,	// ����
	Taunt			= 0x01 << 7,	// ����
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
