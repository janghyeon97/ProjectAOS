// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBarBase.h"
#include "UW_HPBar.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UUW_HPBar : public UUserWidgetBarBase
{
	GENERATED_BODY()
	
public:
    void SetMaxHP(float InMaxHP);

    void InitializeHPBarWidget(TWeakObjectPtr<class UStatComponent> NewStatComponent);

    UFUNCTION()
    void OnMaxHPChanged(float InOldMaxHP, float InNewMaxHP);

    UFUNCTION()
    void OnCurrentHPChanged(float InOldHP, float InNewHP);

protected:
    virtual void NativeConstruct() override;
};
