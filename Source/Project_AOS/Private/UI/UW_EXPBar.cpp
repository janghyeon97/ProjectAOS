// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_EXPBar.h"
#include "Components/ProgressBar.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"

void UUW_EXPBar::SetMaxEXP(float InMaxEXP)
{
	SetMaxFigure(InMaxEXP);
}

void UUW_EXPBar::InitializeEXPBarWidget(TWeakObjectPtr<class UStatComponent> NewStatComponent)
{
	OnCurrentEXPChanged(0, NewStatComponent->GetCurrentEXP());
}

void UUW_EXPBar::OnMaxEXPChanged(float InOldMaxEXP, float InNewMaxEXP)
{
	SetMaxFigure(InNewMaxEXP);

	OnCurrentEXPChanged(0, InNewMaxEXP);
}

void UUW_EXPBar::OnCurrentEXPChanged(float InOldEXP, float InNewEXP)
{
    if (true == ::IsValid(Bar))
    {
        if (KINDA_SMALL_NUMBER < MaxFigure)
        {
            Bar->SetPercent(InNewEXP / MaxFigure);
        }
        else
        {
            Bar->SetPercent(0.f);
        }
    }
}

void UUW_EXPBar::NativeConstruct()
{
    Super::NativeConstruct();

    ACharacterBase* OwningCharacter = Cast<ACharacterBase>(OwningActor);
    if (true == ::IsValid(OwningCharacter))
    {
        OwningCharacter->SetWidget(this);
    }
}
