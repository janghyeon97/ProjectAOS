// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "UserWidgetBarBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UUserWidgetBarBase : public UUserWidgetBase
{
	GENERATED_BODY()
	
public:
    UUserWidgetBarBase(const FObjectInitializer& ObjectInitializer);

    void SetMaxFigure(float InMaxFigure);

protected:
    virtual void NativeConstruct() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_Bar", Meta = (BindWidget))
    TObjectPtr<class UProgressBar> Bar;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_Bar")
    float MaxFigure;
};
