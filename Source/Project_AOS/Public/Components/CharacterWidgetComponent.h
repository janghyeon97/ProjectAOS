// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "CharacterWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UCharacterWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	virtual void InitWidget() override;

	virtual void InitializeWidget();

};
