// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdControls/CrowdControlEffectBase.h"
#include "StunEffect.generated.h"

struct FAbilityDetails;

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UStunEffect : public UCrowdControlEffectBase
{
	GENERATED_BODY()
	
public:
	virtual void ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent = 0.0f) override;
	virtual void RemoveEffect() override;
	virtual void ReturnEffect() override;
	virtual void Reset() override;

	virtual float GetDuration() const override;
	virtual float GetPercent() const override;

private:
	TArray<FAbilityDetails*> DisabledAbilities;
};
