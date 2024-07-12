// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBarBase.h"
#include "UW_MPBar.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UUW_MPBar : public UUserWidgetBarBase
{
	GENERATED_BODY()
	
public:
    void SetMaxMP(float InMaxHP);

    void InitializeMPBarWidget(TWeakObjectPtr<class UStatComponent> NewStatComponent);


    UFUNCTION()
    void OnMaxMPChanged(float InOldMaxHP, float InNewMaxHP);

    UFUNCTION()
    void OnCurrentMPChanged(float InOldHP, float InNewHP);

protected:
    virtual void NativeConstruct() override;
};
