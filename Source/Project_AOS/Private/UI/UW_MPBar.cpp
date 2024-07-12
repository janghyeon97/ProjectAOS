// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_MPBar.h"
#include "Components/ProgressBar.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"

void UUW_MPBar::SetMaxMP(float InMaxMP)
{
    SetMaxFigure(InMaxMP);
}

void UUW_MPBar::InitializeMPBarWidget(TWeakObjectPtr<UStatComponent> NewStatComponent)
{
    OnCurrentMPChanged(0, NewStatComponent->GetCurrentMP());
}

void UUW_MPBar::OnMaxMPChanged(float InOldMaxMP, float InNewMaxMP)
{
    SetMaxFigure(InNewMaxMP);

    OnCurrentMPChanged(0, InNewMaxMP);
}

void UUW_MPBar::OnCurrentMPChanged(float InOldMP, float InNewMP)
{
    if (true == ::IsValid(Bar))
    {
        if (KINDA_SMALL_NUMBER < MaxFigure)
        {
            Bar->SetPercent(InNewMP / MaxFigure);
        }
        else
        {
            Bar->SetPercent(0.f);
        }
    }
}

void UUW_MPBar::NativeConstruct()
{
    Super::NativeConstruct();

    ACharacterBase* OwningCharacter = Cast<ACharacterBase>(OwningActor);
    if (true == ::IsValid(OwningCharacter))
    {
        OwningCharacter->SetWidget(this);
    }
}
