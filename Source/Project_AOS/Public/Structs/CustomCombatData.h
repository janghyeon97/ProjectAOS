// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "Structs/EnumAbilityID.h"
#include "Structs/EnumDamageType.h"
#include "Structs/EnumAttackEffect.h"
#include "Structs/EnumAbilityEffect.h"
#include "Structs/EnumCrowdControl.h"
#include "CrowdControls/CrowdControlEffectBase.h"
#include "CustomCombatData.generated.h"

USTRUCT(BlueprintType)
struct FCrowdControlInformation
{
	GENERATED_BODY()

public:
	FCrowdControlInformation() : Type(EBaseCrowdControl::None), Duration(0), Percent(0) {}
	FCrowdControlInformation(EBaseCrowdControl InCrowdControl, float InDuration, float InPercent)
		: Type(InCrowdControl)
		, Duration(InDuration)
		, Percent(InPercent)
		, Class(nullptr)
		{}
	FCrowdControlInformation(UCrowdControlEffectBase* CrowdControlClass, float InDuration, float InPercent)
		: Class(CrowdControlClass)
		, Duration(InDuration)
		, Percent(InPercent)
		, Type(EBaseCrowdControl::None)
	{}


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBaseCrowdControl Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCrowdControlEffectBase* Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-100.0", ClampMax = "100.0", uIMin = "-100.0", uIMax = "100.0"))
	float Percent;
};


USTRUCT(BlueprintType)
struct FDamageInformation
{
	GENERATED_BODY()

public:
	FDamageInformation()
	{
		AbilityID = EAbilityID::None;
		PhysicalDamage = 0.f;
		MagicDamage = 0.f;
		TrueDamage = 0.f;
		DamageType = EDamageType::None;
		AttackEffect = EAttackEffect::None;
	}

	void AddDamage(EDamageType InDamageType, const float DamageAmount)
	{
		switch (InDamageType)
		{
		case EDamageType::Physical:
			EnumAddFlags(DamageType, EDamageType::Physical);
			PhysicalDamage += DamageAmount;
			break;
		case EDamageType::Magic:
			EnumAddFlags(DamageType, EDamageType::Magic);
			MagicDamage += DamageAmount;
			break;
		case EDamageType::TrueDamage:
			EnumAddFlags(DamageType, EDamageType::TrueDamage);
			TrueDamage += DamageAmount;
			break;
		case EDamageType::Critical:
			EnumAddFlags(DamageType, EDamageType::Critical);
			PhysicalDamage += DamageAmount;
			break;
		default:
			break;
		}
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbilityID AbilityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicalDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MagicDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrueDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageType DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAttackEffect AttackEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCrowdControlInformation> CrowdControls;
};

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UCustomCombatData : public UObject
{
	GENERATED_BODY()
	
};
