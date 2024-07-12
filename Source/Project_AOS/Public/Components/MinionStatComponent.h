// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StatComponent.h"
#include "Structs/EnumMinionType.h"
#include "MinionStatComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UMinionStatComponent : public UStatComponent
{
	GENERATED_BODY()
	
public:
    UMinionStatComponent();

    virtual void InitializeStatComponent(EMinionType MinionType);

private:
    void UpdateStatsBasedOnElapsedTime(EMinionType MinionType);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time", meta = (AllowPrivateAccess))
    float ElapsedTime;
};
