// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "DamageNumberWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UDamageNumberWidget : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Damage")
	void SetDamageAmount(const float DamageAmount, const FLinearColor InColor, const float TextScale);

	// FadeOut �ִϸ��̼� ��ȯ �Լ�
	UFUNCTION(BlueprintCallable, Category = "Damage")
	UWidgetAnimation* GetFadeOutAnimation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage", meta = (BindWidget))
	TObjectPtr<class UTextBlock> DamageText;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeOutAnimation;
};
