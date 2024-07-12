// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_HPBar.h"
#include "Components/ProgressBar.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"

void UUW_HPBar::SetMaxHP(float InMaxHP)
{
    SetMaxFigure(InMaxHP);
}

void UUW_HPBar::InitializeHPBarWidget(TWeakObjectPtr<UStatComponent> NewStatComponent)
{
    OnCurrentHPChanged(0, NewStatComponent->GetCurrentHP());
}

void UUW_HPBar::OnMaxHPChanged(float InOldMaxHP, float InNewMaxHP)
{
    SetMaxFigure(InNewMaxHP);

    OnCurrentHPChanged(0, InNewMaxHP);
}

void UUW_HPBar::OnCurrentHPChanged(float InOldHP, float InNewHP)
{
    if (true == ::IsValid(Bar))
    {
        if (KINDA_SMALL_NUMBER < MaxFigure)
        {
            Bar->SetPercent(InNewHP / MaxFigure);
        }
        else
        {
            Bar->SetPercent(0.f);
        }
    }
}

void UUW_HPBar::NativeConstruct()
{
    Super::NativeConstruct();

    ACharacterBase* OwningCharacter = Cast<ACharacterBase>(OwningActor);
    if (true == ::IsValid(OwningCharacter))
    {
        OwningCharacter->SetWidget(this);
    }   
}
