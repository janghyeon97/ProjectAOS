// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "HealingPotion.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AHealingPotion : public AItem
{
	GENERATED_BODY()
	
public:
	AHealingPotion();

protected:
	virtual void Use(class AAOSPlayerState* PlayerState) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float HealthRegenerationIncrement; // 회복량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float HealingDuration; // 회복 지속 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float HealingInterval; // 회복 간격
};
