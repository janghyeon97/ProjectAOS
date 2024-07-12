// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBarBase.h"
#include "UW_EXPBar.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UUW_EXPBar : public UUserWidgetBarBase
{
	GENERATED_BODY()
	
public:
    void SetMaxEXP(float InMaxEXP);

    void InitializeEXPBarWidget(TWeakObjectPtr<class UStatComponent> NewStatComponent);


    UFUNCTION()
    void OnMaxEXPChanged(float InOldMaxEXP, float InNewMaxEXP);

    UFUNCTION()
    void OnCurrentEXPChanged(float InOldEXP, float InNewEXP);

protected:
    virtual void NativeConstruct() override;
};
