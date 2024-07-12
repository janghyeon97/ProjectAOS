// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "ManaPotion.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AManaPotion : public AItem
{
	GENERATED_BODY()
	
public:
	AManaPotion();

protected:
	virtual void Use(class AAOSPlayerState* PlayerState) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float ManaRegenerationIncrement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float ManaRegenDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float ManaRegenInterval; // 회복 간격
};
