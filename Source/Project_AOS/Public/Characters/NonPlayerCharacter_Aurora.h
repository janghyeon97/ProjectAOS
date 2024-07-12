// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/NonPlayerCharacterBase.h"
#include "NonPlayerCharacter_Aurora.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API ANonPlayerCharacter_Aurora : public ANonPlayerCharacterBase
{
	GENERATED_BODY()
	
public:
	ANonPlayerCharacter_Aurora();

	virtual void BeginPlay() override;

protected:
	virtual void Ability_Q() override;

	virtual void Ability_E() override;

	virtual void Ability_LMB() override;

	virtual void OnPreDamageReceived(float FinalDamage) override;

	virtual void MontageEnded(UAnimMontage* Montage, bool bIsInterrupt) override;

	void CanNextCombo();
};
