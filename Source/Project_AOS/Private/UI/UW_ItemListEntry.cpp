#include "UI/UW_ItemListEntry.h"
#include "UI/UW_ItemShop.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Game/AOSPlayerState.h"
#include "Game/AOSGameState.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

// Initialization Functions
void UUW_ItemListEntry::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    bDisplaySubItems = true;
    ParentNode = nullptr;
    ChildNodes = TArray<UUW_ItemListEntry*>();

    MaterialRef = ItemImage->GetDynamicMaterial();

    //ItemImage->SetVisibility(ESlateVisibility::Hidden);
    //ItemBorder->SetVisibility(ESlateVisibility::Hidden);
    //PriceText->SetVisibility(ESlateVisibility::Collapsed);
    //ItemButton->SetIsEnabled(false);
}

void UUW_ItemListEntry::NativeConstruct()
{
    Super::NativeConstruct();

    ItemButton->OnClicked.AddDynamic(this, &ThisClass::OnButtonClicked);
    ItemButton->OnHovered.AddDynamic(this, &ThisClass::OnMouseHovered);
    ItemButton->OnUnhovered.AddDynamic(this, &ThisClass::OnMouseUnHovered);
}

// Node Setup Functions
void UUW_ItemListEntry::SetupNode(const int32 NewItemID, UTexture* NewImage, const FString& NewPrice)
{
    ItemID = NewItemID;

    if (IsEmptyNode())
    {
        SetEmptyNode();
    }
    else
    {
        SetItemNode(NewImage, NewPrice);
    }
}

bool UUW_ItemListEntry::IsEmptyNode() const
{
    return ItemID == 0;
}

void UUW_ItemListEntry::SetEmptyNode()
{
    ItemImage->SetVisibility(ESlateVisibility::Hidden);
    ItemBorder->SetVisibility(ESlateVisibility::Hidden);
    PriceText->SetVisibility(ESlateVisibility::Collapsed);
    ItemButton->SetIsEnabled(false);
}

void UUW_ItemListEntry::SetItemNode(UTexture* NewImage, const FString& NewPrice)
{
    if (NewImage != nullptr)
    {
        InitializeMaterial();
        if (MaterialRef != nullptr)
        {
            MaterialRef->SetTextureParameterValue(FName("Image"), NewImage);
        }

        ItemImage->SetVisibility(ESlateVisibility::Visible);
        ItemBorder->SetVisibility(ESlateVisibility::Visible);
        ItemButton->SetIsEnabled(true);
    }

    if (!NewPrice.IsEmpty())
    {
        PriceText->SetText(FText::FromString(NewPrice));
        PriceText->SetVisibility(ESlateVisibility::Visible);

        bDisplaySubItems = true;
    }
}

void UUW_ItemListEntry::InitializeMaterial()
{
    if (MaterialRef == nullptr)
    {
        MaterialRef = ItemImage->GetDynamicMaterial();
    }
}

// UI Update Functions
void UUW_ItemListEntry::SetItemSelected(bool bIsSelected)
{
    if (ItemBorder)
    {
        ItemBorder->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UUW_ItemListEntry::UpdateItemPrice(const int32 Price)
{
    FString PriceString = FString::Printf(TEXT("%d"), Price);
    PriceText->SetText(FText::FromString(PriceString));
    PriceText->SetVisibility(ESlateVisibility::Visible);
}

void UUW_ItemListEntry::UpdateItemImage(UTexture* NewTexture)
{
    if (!NewTexture)
    {
        return;
    }

    MaterialRef->SetTextureParameterValue(FName("Image"), NewTexture);
}

void UUW_ItemListEntry::UpdatePriceVisibility(bool bVisibility)
{
    if (PriceText)
    {
        PriceText->SetVisibility(bVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

// Interaction and Event Handling Functions
void UUW_ItemListEntry::BindItemShopWidget(UUW_ItemShop* Widget)
{
    if (!Widget)
    {
        return;
    }

    ItemShop = Widget;
}

void UUW_ItemListEntry::OnButtonClicked()
{
    if (!ItemShop.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemListEntry::NativeOnMouseButtonDown] Failed to find UUW_ItemShop in outer hierarchy"));
        return;
    }

   
    if (ItemID != 0)
    {
        if (bDisplaySubItems)
        {
            ItemShop->DisplayItemWithSubItems(ItemID);
            ItemShop->DisplayItemDescription(ItemID);
            ItemShop->SetSelectedItem(this);
        }
        else
        {
            ItemShop->DisplayItemDescription(ItemID);
            //ItemShop->SetSelectedItem(this);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemListEntry::OnButtonClicked] Invalid ItemID: %d"), ItemID);
    }
}

void UUW_ItemListEntry::OnMouseHovered()
{
    bMouseHovered = true;
}

void UUW_ItemListEntry::OnMouseUnHovered()
{
    bMouseHovered = false;
}

FReply UUW_ItemListEntry::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (bMouseHovered)
        {
            if (!ItemShop.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("[UUW_ItemListEntry::NativeOnMouseButtonDown] Failed to find UUW_ItemShop in outer hierarchy"));
                return FReply::Handled();
            }

            ItemShop->PurchaseItem(ItemID);
        }

        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
