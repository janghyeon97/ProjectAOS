// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/ItemData.h"
#include "UW_ItemListEntry.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UUW_ItemListEntry : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    void SetupNode(const int32 NewItemID, UTexture* NewIamge, const FString& NewPrice);
    void SetItemSelected(bool bIsSelected);
    void BindItemShopWidget(class UUW_ItemShop* Widget);

    void UpdateItemPrice(const int32 Price);
    void UpdateItemImage(UTexture* NewTexure);
    void UpdatePriceVisibility(bool bVisibility);
    void UpdateDisplaySubItems(bool bShowSubItems) { bDisplaySubItems = bShowSubItems; };


    UFUNCTION()
    void OnButtonClicked();

    UFUNCTION()
    void OnMouseHovered();

    UFUNCTION()
    void OnMouseUnHovered();

protected:
    bool IsEmptyNode() const;
    void SetEmptyNode();
    void SetItemNode(UTexture* NewImage, const FString& NewPrice);
    void InitializeMaterial();
	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TWeakObjectPtr<class UUW_ItemShop> ItemShop;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UImage> ItemImage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UImage> ItemBorder;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> PriceText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UButton> ItemButton;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UUW_ItemListEntry* ParentNode;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    TArray<UUW_ItemListEntry*> ChildNodes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess))
    int32 ItemID = 0;

private:
    UMaterialInstanceDynamic* MaterialRef = nullptr;

    bool bMouseHovered = false;
    bool bDisplaySubItems = false;
};
