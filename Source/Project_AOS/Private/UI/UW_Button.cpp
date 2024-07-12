// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_Button.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

UUW_Button::UUW_Button(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

void UUW_Button::NativeConstruct()
{
	Super::NativeConstruct();

	Button->OnHovered.AddDynamic(this, &UUW_Button::PlayHoverAnimation);
	Button->OnUnhovered.AddDynamic(this, &UUW_Button::PlayUnHoverAnimation);
}

void UUW_Button::PlayHoverAnimation()
{
	PlayAnimation(HoverAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, false);
}

void UUW_Button::PlayUnHoverAnimation()
{
	PlayAnimation(HoverAnimation, 0.f, 1, EUMGSequencePlayMode::Reverse, 1.f, false);
}

void UUW_Button::PlayConstructAnimation()
{
	PlayAnimation(ConstructAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, false);
}

void UUW_Button::SetButtonText(FString InText)
{
	ButtonText->SetText(FText::FromString(InText));
}