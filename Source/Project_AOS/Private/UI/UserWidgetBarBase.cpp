// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgetBarBase.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"

UUserWidgetBarBase::UUserWidgetBarBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) 
{

}


void UUserWidgetBarBase::SetMaxFigure(float InMaxFigure)
{
    if (InMaxFigure < KINDA_SMALL_NUMBER)
    {
        MaxFigure = 0.f;
        return;
    }

    MaxFigure = InMaxFigure;
}

void UUserWidgetBarBase::SetBorderColor(FLinearColor InColor)
{
    Border->SetBrushColor(InColor);
}

void UUserWidgetBarBase::NativeConstruct()
{
    Super::NativeConstruct();

}
