// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Button.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UUW_Button : public UUserWidget
{
	GENERATED_BODY()

public:
	UUW_Button(const FObjectInitializer &ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void PlayHoverAnimation();

	UFUNCTION()
	void PlayUnHoverAnimation();

	UFUNCTION()
	void PlayConstructAnimation();

	UFUNCTION()
	void SetButtonText(FString InText);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UButton> Button;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UImage> ButtonBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UImage> ButtonStroke;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> ButtonText;

	// User Widget Animation ---------------------------------
	UPROPERTY(EditAnywhere, Transient, Category = "UW_Button", meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> HoverAnimation;

	UPROPERTY(EditAnywhere, Transient, Category = "UW_Button", meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> ConstructAnimation;
};
