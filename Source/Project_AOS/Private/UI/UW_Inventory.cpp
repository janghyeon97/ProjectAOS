// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_Inventory.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"


void UUW_Inventory::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    ItemImages.SetNum(InventorySize);
    ItemButtons.SetNum(InventorySize);
    ItemCounts.SetNum(InventorySize);

    InitializeWidgets();

    for (auto& ItemImage : ItemImages)
    {
        MaterialRef.Add(ItemImage->GetDynamicMaterial());
    }

    DefaultTexture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL, TEXT("/Game/ProjectAOS/UI/Lobby/Image/Image_Transparent.Image_Transparent")));
}

void UUW_Inventory::InitializeWidgets()
{
    for (int32 i = 0; i < InventorySize; ++i)
    {
        FString ImageName = FString::Printf(TEXT("ItemImage_%d"), i + 1);
        FString ButtonName = FString::Printf(TEXT("ItemButton_%d"), i + 1);
        FString CountName = FString::Printf(TEXT("ItemCount_%d"), i + 1);

        if (UImage* FoundImage = Cast<UImage>(GetWidgetFromName(FName(*ImageName))))
        {
            ItemImages[i] = FoundImage;
        }

        if (UButton* FoundButton = Cast<UButton>(GetWidgetFromName(FName(*ButtonName))))
        {
            ItemButtons[i] = FoundButton;
        }

        if (UTextBlock* FoundCount = Cast<UTextBlock>(GetWidgetFromName(FName(*CountName))))
        {
            ItemCounts[i] = FoundCount;
        }
    }
}

void UUW_Inventory::UpdateItemImage(UTexture* NewTexture, int32 Index)
{
    if (ItemImages.IsValidIndex(Index) && ItemImages[Index])
    {
        MaterialRef[Index]->SetTextureParameterValue("Tex", NewTexture ? NewTexture : DefaultTexture);
    }
}

void UUW_Inventory::UpdateItemCountText(int32 Count, int32 Index)
{
    if (ItemCounts.IsValidIndex(Index) && ItemCounts[Index])
    {
        if (Count <= 1)
        {
            ItemCounts[Index]->SetText(FText::GetEmpty());
            return;
        }

        FString ItemCountString = FString::Printf(TEXT("%d"), Count);
        ItemCounts[Index]->SetText(FText::FromString(ItemCountString));
    }
}

void UUW_Inventory::UpdateCurrencyText(FText NewText)
{
    if (!NewText.IsEmptyOrWhitespace() && CurrencyText)
    {
        CurrencyText->SetText(NewText);
    }
}

