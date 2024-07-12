// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterWidgetComponent.h"
#include "UI/UserWidgetBase.h"

void UCharacterWidgetComponent::InitWidget()
{
	Super::InitWidget();

	UUserWidgetBase* UserWidget = Cast<UUserWidgetBase>(GetWidget());
	if (::IsValid(UserWidget))
	{
		UserWidget->SetOwningActor(GetOwner());
	}
}

void UCharacterWidgetComponent::InitializeWidget()
{
	
}
