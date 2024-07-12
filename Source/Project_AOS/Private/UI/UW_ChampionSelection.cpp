// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ChampionSelection.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UUW_ChampionSelection::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_ChampionSelection::InitializeListEntry()
{
	MaterialRef = ChampionImage->GetDynamicMaterial();
}

void UUW_ChampionSelection::UpdateChampionNameText(FString InString)
{
	ChampionNameText->SetText(FText::FromString(InString));
}

void UUW_ChampionSelection::UpdateChampionNameColor(FLinearColor InColor)
{
	ChampionNameText->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::UpdateChampionPositionText(FString InString)
{
	ChampionPositionText->SetText(FText::FromString(InString));
}

void UUW_ChampionSelection::UpdateChampionPositionColor(FLinearColor InColor)
{
	ChampionPositionText->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::UpdatePlayerNameText(FString InString)
{
	PlayerNameText->SetText(FText::FromString(InString));
}

void UUW_ChampionSelection::UpdatePlayerNameColor(FLinearColor InColor)
{
	PlayerNameText->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::UpdateBorderImageColor(FLinearColor InColor)
{
	BorderImage->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::SetChampionInfoVisibility(bool bVisibility)
{
	if (bVisibility)
	{
		ChampionNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
		ChampionPositionText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		ChampionNameText->SetVisibility(ESlateVisibility::Collapsed);
		ChampionPositionText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUW_ChampionSelection::UpdateCampionImage(UTexture* InTexture)
{
	MaterialRef = ChampionImage->GetDynamicMaterial();
	MaterialRef->SetTextureParameterValue(FName("Image"), InTexture);
}
