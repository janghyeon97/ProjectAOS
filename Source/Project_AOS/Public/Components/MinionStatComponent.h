// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StatComponent.h"
#include "Structs/EnumMinionType.h"
#include "MinionStatComponent.generated.h"


class ICharacterDataProviderInterface;

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UMinionStatComponent : public UStatComponent
{
	GENERATED_BODY()
	
public:
    UMinionStatComponent();

    virtual void InitStatComponent(ICharacterDataProviderInterface* DataProvider, const FName& RowName) override;

private:
    void UpdateStatsBasedOnElapsedTime(const FName& RowName);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time", meta = (AllowPrivateAccess))
    float ElapsedTime;
};
