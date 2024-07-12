#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "Item/ItemData.h"
#include "UW_ItemShop.generated.h"

class AAOSGameState;
class AAOSPlayerState;
class UStatComponent;
class UUW_ItemListEntry;
class UUniformGridPanel;
class UStackBox;
class UImage;
class UTextBlock;
class URichTextBlock;
class UStackBoxSlot;
class UVerticalBox;
class UHorizontalBox;
class UHorizontalBoxSlot;

/**
 *
 */
UCLASS()
class PROJECT_AOS_API UUW_ItemShop : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	// Constructor
	UUW_ItemShop(const FObjectInitializer& ObjectInitializer);

	// Overridden functions
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Public functions
	void InitializeItemList();
	void BindPlayerState(AAOSPlayerState* InPlayerState);
	void PurchaseItem(const int32 ItemID);
	void DisplayItemWithSubItems(int32 ItemID);
	void DisplayItemDescription(int32 ItemID);
	void SetSelectedItem(UUW_ItemListEntry* NewSelectedItem);
	void PlaySound(USoundBase* Sound);
	//void AdjustAndDisplayItemPrice(UUW_ItemListEntry* Entry, FItemInformation* ItemInfo);
	//void ApplyDiscount(FItemInformation* ItemInfo, int32& FinalPrice, TArray<int32>& PlayerInventoryItemIDs);

	TWeakObjectPtr<AAOSGameState> GetGameState() { return GameState; }

	UFUNCTION()
	void OnItemPurchased(int32 ItemID);

private:
	// Private functions
	UHorizontalBox* CreateRootHorizontalBox();
	UUniformGridPanel* GetOrCreateClassificationPanel(const FString& ClassificationString);

	void SetupStackBoxSlot(UStackBoxSlot* NewSlot, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);
	void SetupHorizontalBoxSlot(UHorizontalBoxSlot* NewSlot, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);

	void AddClassifiedItemEntry(const FItemInformation& Item);
	void AddNodesBreadthFirst(UUW_ItemListEntry* RootNode);
	void AddChildNodesRecursively(UUW_ItemListEntry* ParentNode, UHorizontalBox* ParentBox, int32 Depth);
	void AddItemDescription(const FItemInformation& ItemInfo, UStatComponent* StatComponent);
	void AddTopLevelBoxToItemHierarchy(UHorizontalBox* TopLevelBox);
	void AddNodeToStackBox(UStackBox* ParentBox, UWidget* Node, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);
	void AddNodeToHorizontalBox(UHorizontalBox* ParentBox, UWidget* Node, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);

	UUW_ItemListEntry* CreateNodeHierarchy(const FItemInformation& NodeInfo);
	UUW_ItemListEntry* FindLoadedItemByID(int32 ItemID) const;
	UUW_ItemListEntry* CreateCachedNode(const FItemInformation& NodeInfo);
	UUW_ItemListEntry* GetOrCreateItemHierarchy(const FItemInformation& ItemInfo);
	FItemInformation* GetItemInfoByID(int32 ItemID);

protected:
	// Properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemShop", Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> ItemListEntryClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemShop", Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> TreeNodeWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemShop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ItemDescriptionLineClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UStackBox> ItemList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UStackBox> ItemHierarchyBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UStackBox> ItemDescriptionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UImage> CoinImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemPrice;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemShop")
	UDataTable* RichTextStyleSet;

private:
	// Private member variables
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ItemShop", meta = (AllowPrivateAccess))
	TArray<TObjectPtr<UUW_ItemListEntry>> LoadedItems;

	TWeakObjectPtr<AAOSGameState> GameState;
	TWeakObjectPtr<AAOSPlayerState> PlayerState;
	TWeakObjectPtr<UUW_ItemListEntry> SelectedItem;

	TMap<FString, UUniformGridPanel*> ClassificationPanels;
	TMap<FString, int32> ClassificationRows;
	TMap<FString, int32> ClassificationColumns;

	TMap<int32, TArray<TWeakObjectPtr<UUW_ItemListEntry>>> ItemHierarchyCache;
	TMap<int32, TArray<class UUW_ItemDescriptionLine*>> ItemDescriptionCache;

	UMaterialInstanceDynamic* ItemImageRef = nullptr;

	const int32 MaxColumn = 10;
	int32 BoxIndex = 0;
	int32 NodeIDCounter;
};
