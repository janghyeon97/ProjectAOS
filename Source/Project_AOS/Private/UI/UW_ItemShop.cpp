#include "UI/UW_ItemShop.h"
#include "UI/UW_ItemListEntry.h"
#include "UI/TreeNodeWidget.h"
#include "UI/UW_ItemDescriptionLine.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/StackBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/RichTextBlock.h"
#include "Components/TreeView.h"
#include "Components/TextBlock.h"
#include "Components/StackBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/UniformGridSlot.h"
#include "Components/StatComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Game/AOSGameState.h"
#include "Game/AOSPlayerState.h"
#include "Item/Item.h"
#include "Item/ItemData.h"
#include "Structs/StructItemAbility.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"

// Constructor and Initialization
UUW_ItemShop::UUW_ItemShop(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTable(TEXT("/Game/ProjectAOS/DataTables/DT_RichTextStyles.DT_RichTextStyles"));
    if (DataTable.Succeeded())
    {
        RichTextStyleSet = DataTable.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load data table: /Game/ProjectAOS/DataTables/DT_RichTextStyles.DT_RichTextStyles"));
    }
}

void UUW_ItemShop::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    CoinImage->SetVisibility(ESlateVisibility::Hidden);
    ItemImage->SetVisibility(ESlateVisibility::Hidden);
    ItemImageRef = ItemImage->GetDynamicMaterial();
}

void UUW_ItemShop::NativeConstruct()
{
    Super::NativeConstruct();

    GameState = Cast<AAOSGameState>(UGameplayStatics::GetGameState(GetWorld()));
    if (!GameState.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ItemShop::NativeConstruct] GameState is invalid."));
    }

    InitializeItemList();
}

// Event Handlers
FReply UUW_ItemShop::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // Handle left mouse button click
        return FReply::Handled();
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        // Handle right mouse button click
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUW_ItemShop::BindPlayerState(AAOSPlayerState* InPlayerState)
{
    if (::IsValid(InPlayerState))
    {
        PlayerState = InPlayerState;

        PlayerState->OnItemPurchased.AddDynamic(this, &ThisClass::OnItemPurchased);
    }
}

// Item List Initialization and Management
void UUW_ItemShop::InitializeItemList()
{
    if (!GameState.IsValid())
    {
        return;
    }

    // Retrieve loaded items
    TArray<FItemInformation> Items = GameState->GetLoadedItems();
    if (Items.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ItemShop::InitializeItemList] No items in LoadedItems"));
        return;
    }

    // Add items
    for (const auto& Item : Items)
    {
        AddClassifiedItemEntry(Item);
    }
}

/**
 * AddClassifiedItemEntry �Լ��� �־��� ������ ������ ������� �з��� ������ �׸��� UI�� �߰��մϴ�.
 * ������ ��Ͽ� �׸��� �߰��ϰ�, ������ �з� �гο� �׸��� ��ġ�մϴ�.
 *
 * @param FItemInformation - ������ ���� ����ü
 */
void UUW_ItemShop::AddClassifiedItemEntry(const FItemInformation& Item)
{
    if (!ItemListEntryClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::AddClassifiedItemEntry] ItemListEntryClass is not valid."));
        return;
    }

    const FString ClassificationString = Item.ClassificationToString();
    UUniformGridPanel* ClassificationPanel = GetOrCreateClassificationPanel(ClassificationString);

    if (!ClassificationPanel)
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::AddClassifiedItemEntry] Failed to get or create ClassificationPanel for %s."), *ClassificationString);
        return;
    }

    UUW_ItemListEntry* Entry = CreateWidget<UUW_ItemListEntry>(this, ItemListEntryClass);
    if (!Entry)
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::AddClassifiedItemEntry] Failed to create ItemListEntry widget."));
        return;
    }

    // Initialize the item entry
    Entry->SetupNode(Item.ItemID, Item.Icon, FString::Printf(TEXT("%d"), Item.Price));
    Entry->BindItemShopWidget(this);

    LoadedItems.Add(Entry);

    // Retrieve row and column for this classification
    int32& Row = ClassificationRows.FindOrAdd(ClassificationString);
    int32& Column = ClassificationColumns.FindOrAdd(ClassificationString);

    // Add entry to the classification panel
    ClassificationPanel->AddChildToUniformGrid(Entry, Row, Column);

    // Update column and row
    if (++Column >= MaxColumn)
    {
        Column = 0;
        Row++;
    }
}

/**
 * GetOrCreateClassificationPanel �Լ��� �־��� �з� ���ڿ��� �ش��ϴ� UUniformGridPanel�� ��ȯ�մϴ�.
 * �г��� �̹� �����ϴ� ��� �ش� �г��� ��ȯ�ϰ�, �������� �ʴ� ��� ���� �����Ͽ� ��ȯ�մϴ�.
 *
 * @param ClassificationString - �з� ���ڿ�
 * @return UUniformGridPanel ��ü (�з� �г�)
 */
UUniformGridPanel* UUW_ItemShop::GetOrCreateClassificationPanel(const FString& ClassificationString)
{
    // Check if ClassificationPanel already exists and return
    UUniformGridPanel** FoundPanel = ClassificationPanels.Find(ClassificationString);
    if (FoundPanel)
    {
        return *FoundPanel;
    }

    // Create and initialize VerticalBox
    UVerticalBox* VerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(
        UVerticalBox::StaticClass(),
        FName(FString::Format(TEXT("ChildrenVerticalBox{0}"), { BoxIndex++ }))
    );

    // Add text block for classification
    UTextBlock* ClassificationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ClassificationText"));
    ClassificationText->SetText(FText::FromString(ClassificationString));

    FSlateFontInfo FontInfo = ClassificationText->GetFont();
    FontInfo.Size = 20.f;
    FontInfo.TypefaceFontName = FName("Regular");
    ClassificationText->SetFont(FontInfo);

    VerticalBox->AddChildToVerticalBox(ClassificationText);

    // Create and add UniformGridPanel to VerticalBox
    UUniformGridPanel* ClassificationPanel = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("ClassificationPanel"));
    ClassificationPanel->SetSlotPadding(FMargin(5.f));

    VerticalBox->AddChildToVerticalBox(ClassificationPanel);
    ItemList->AddChildToStackBox(VerticalBox);

    // Add ClassificationPanel and related data
    ClassificationPanels.Add(ClassificationString, ClassificationPanel);
    ClassificationRows.Add(ClassificationString, 0);
    ClassificationColumns.Add(ClassificationString, 0);

    return ClassificationPanel;
}


// ----------------------------------------------------------------------------------------------------------

void UUW_ItemShop::PurchaseItem(const int32 ItemID)
{
    if (ItemID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::PurchaseItem] Invalid ItemID: %d"), ItemID);
        return;
    }

    if (!PlayerState.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::PurchaseItem] PlayerState is not valid"));
        return;
    }

    PlayerState->ServerPurchaseItem(ItemID);
}

void UUW_ItemShop::OnItemPurchased(int32 ItemID)
{
    FString SoundPath = FString::Printf(TEXT("/Game/ProjectAOS/UI/ItemShop/Audio_PurchaseSuccess.Audio_PurchaseSuccess"));
    USoundBase* PurchaseSuccessSound = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), NULL, *SoundPath));

    // ������ ���� �� UI ������Ʈ
    if (SelectedItem.IsValid())
    {
        DisplayItemWithSubItems(SelectedItem->ItemID);
    }
   
    // ���� �Ϸ� ���� ���
    PlaySound(PurchaseSuccessSound);

    UE_LOG(LogTemp, Log, TEXT("[UUW_ItemShop::OnItemPurchased] OnItemPurchased"));
}

/*
void UUW_ItemShop::AdjustAndDisplayItemPrice(UUW_ItemListEntry* Entry, FItemInformation* ItemInfo)
{
    if (!Entry || !ItemInfo)
    {
        return;
    }

    int32 FinalPrice = ItemInfo->Price;

    if (!PlayerState.IsValid() || !GameState.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::AdjustAndDisplayItemPrice] PlayerState or GameState is invalid."));
        return;
    }

    TArray<int32> PlayerInventoryItemIDs;
    for (int32 i = 0; i < GameState->GetInventory().Inventory.Num(); ++i)
    {
        if (Inventory[i] != nullptr)
        {
        if (::IsValid(OwnedItem))
        {
            PlayerInventoryItemIDs.Add(OwnedItem->ItemID);
        }
    }

    ApplyDiscount(ItemInfo, FinalPrice, PlayerInventoryItemIDs);

    Entry->UpdateItemPrice(FinalPrice);
}

void UUW_ItemShop::ApplyDiscount(FItemInformation* ItemInfo, int32& FinalPrice, TArray<int32>& PlayerInventoryItemIDs)
{
    for (int32 RequiredItemID : ItemInfo->RequiredItems)
    {
        if (PlayerInventoryItemIDs.Contains(RequiredItemID))
        {
            FItemInformation* RequiredItemInfo = GameState->GetItemInfoByID(RequiredItemID);
            if (RequiredItemInfo)
            {
                FinalPrice = FMath::Max(0, FinalPrice - RequiredItemInfo->Price);
                PlayerInventoryItemIDs.RemoveSingle(RequiredItemID);
                ApplyDiscount(RequiredItemInfo, FinalPrice, PlayerInventoryItemIDs);
            }
        }
    }
}
*/


/**
 * DisplayItemDescription �Լ��� �־��� ������ �ε����� ������� �������� ������ UI�� ǥ���մϴ�.
 * ĳ���Ϳ� ���� ������Ʈ�� ������ ������ ������ ������� ������ �߰��մϴ�.
 *
 * @param ItemIndex - ������ �ε���
 */
void UUW_ItemShop::DisplayItemDescription(int32 ItemIndex)
{
    // ĳ���� ��������
    AAOSCharacterBase* Character = Cast<AAOSCharacterBase>(OwningActor);
    if (!::IsValid(Character))
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::DisplaySubItems] Invalid Character"));
        return;
    }

    // ���� ������Ʈ ��������
    UStatComponent* StatComponent = Character->GetStatComponent();
    if (!::IsValid(StatComponent))
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::DisplaySubItems] Invalid StatComponent"));
        return;
    }

    // ������ ���� ��������
    FItemInformation* SubItemInfo = GameState->GetItemInfoByID(ItemIndex);
    if (!SubItemInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ItemShop::DisplayItemDescription] Could not find SubItemInfo for ItemID: %d"), ItemIndex);
        return;
    }

    AddItemDescription(*SubItemInfo, StatComponent);

    // ������ ���� ������Ʈ
    if (ItemImageRef && ItemName && ItemPrice && CoinImage && ItemImage)
    {
        ItemImageRef->SetTextureParameterValue(FName("Image"), SubItemInfo->Icon);
        ItemName->SetText(FText::FromString(SubItemInfo->Name));
        ItemPrice->SetText(FText::FromString(FString::Printf(TEXT("%d"), SubItemInfo->Price)));

        CoinImage->SetVisibility(ESlateVisibility::Visible);
        ItemImage->SetVisibility(ESlateVisibility::Visible);
    }
}


/**
 * AddItemDescription �Լ��� �־��� ������ ������ ���� ������Ʈ�� ����Ͽ� �������� �ɷ� �� ������ UI�� �߰��մϴ�.
 * ���� ������ ����� ���ο� ������ �߰��� �� ĳ�ÿ� �����մϴ�.
 *
 * @param ItemInfo - ������ ����
 * @param StatComponent - ���� ������Ʈ
 */
void UUW_ItemShop::AddItemDescription(const FItemInformation& ItemInfo, UStatComponent* StatComponent)
{
    if (!ItemDescriptionBox)
    {
        return;
    }

    // Clear existing description
    ItemDescriptionBox->ClearChildren();

    TArray<UUW_ItemDescriptionLine*> NewDescriptionLines;

    // Add abilities as text
    for (const FItemAbility& Ability : ItemInfo.Abilities)
    {
        FString AbilityText = FString::Printf(TEXT("%s: %d"), *Ability.AbilityTypeToString(), Ability.AbilityValue);
        UUW_ItemDescriptionLine* DescriptionLine = CreateWidget<UUW_ItemDescriptionLine>(this, ItemDescriptionLineClass);
        if (DescriptionLine)
        {
            DescriptionLine->SetDescriptionText(AbilityText);
            ItemDescriptionBox->AddChildToStackBox(DescriptionLine);
            NewDescriptionLines.Add(DescriptionLine);
        }
    }

    if (!ItemInfo.Description.IsEmpty())
    {
        // Add item description after converting to rich text
        FString RichDescription = ItemInfo.ConvertToRichText(StatComponent);
        UUW_ItemDescriptionLine* DescriptionLine = CreateWidget<UUW_ItemDescriptionLine>(this, ItemDescriptionLineClass);
        if (DescriptionLine)
        {
            DescriptionLine->SetDescriptionText(RichDescription);
            ItemDescriptionBox->AddChildToStackBox(DescriptionLine);
            NewDescriptionLines.Add(DescriptionLine);
        }
    }

    // Cache the new description lines
    ItemDescriptionCache.Add(ItemInfo.ItemID, NewDescriptionLines);
}

// ----------------------------------------------------------------------------------------

// Item Hierarchy Management
void UUW_ItemShop::DisplayItemWithSubItems(int32 ItemIndex)
{
    ItemHierarchyBox->ClearChildren();

    if (!GameState.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::DisplayItemWithSubItems] Invalid GameState"));
        return;
    }

    FItemInformation* ItemInfo = GameState->GetItemInfoByID(ItemIndex);
    if (!ItemInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::DisplayItemWithSubItems] ItemInfo not found for ItemID: %d"), ItemIndex);
        return;
    }

    UUW_ItemListEntry* RootNode = GetOrCreateItemHierarchy(*ItemInfo);
    if (RootNode)
    {
        AddNodesBreadthFirst(RootNode);
        //AdjustAndDisplayItemPrice(RootNode, ItemInfo);
    }
}


/**
 * CreateNodeHierarchy �Լ��� �־��� ������ ����(NodeInfo)�� ���� ��������� ��� ���� ������ �����մϴ�.
 * �� ���� �ڽ� ��带 �����ϸ�, ��� ���� ĳ�ø� ���� �����˴ϴ�.
 *
 * @param NodeInfo - ������ ����
 * @return UUW_ItemListEntry ��ü (���� ������ ��Ʈ ���)
 */
UUW_ItemListEntry* UUW_ItemShop::CreateNodeHierarchy(const FItemInformation& NodeInfo)
{
    // ĳ�ÿ��� ��带 ã�ų� ���� �����Ͽ� ��ȯ
    UUW_ItemListEntry* RootNode = CreateCachedNode(NodeInfo);
    if (!RootNode)
    {
        return nullptr;
    }

    int32 MaxChildCount = 0;
    TArray<UUW_ItemListEntry*> ChildNodes;

    for (const int32 ChildItemID : NodeInfo.RequiredItems)
    {
        FItemInformation* SubItem = GameState->GetItemInfoByID(ChildItemID);
        if (SubItem)
        {
            UUW_ItemListEntry* ChildNode = CreateNodeHierarchy(*SubItem);
            ChildNode->ParentNode = RootNode;
            ChildNodes.Add(ChildNode);
            MaxChildCount = FMath::Max(MaxChildCount, ChildNode->ChildNodes.Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SubItemInfo not found for ItemID: %d"), ChildItemID);
        }
    }

    // �� ��� �߰�
    for (int32 i = 0; i < ChildNodes.Num(); ++i)
    {
        while (ChildNodes[i]->ChildNodes.Num() < MaxChildCount)
        {
            UUW_ItemListEntry* EmptyNode = CreateCachedNode(FItemInformation());
            EmptyNode->ParentNode = ChildNodes[i];
            ChildNodes[i]->ChildNodes.Add(EmptyNode);
        }
        RootNode->ChildNodes.Add(ChildNodes[i]);
    }

    // �� ��� �߰��ؼ� ���� ���߱�
    while (RootNode->ChildNodes.Num() < MaxChildCount)
    {
        UUW_ItemListEntry* EmptyNode = CreateCachedNode(FItemInformation());
        EmptyNode->ParentNode = RootNode;
        RootNode->ChildNodes.Add(EmptyNode);
    }

    return RootNode;
}

/**
 * AddNodesBreadthFirst �Լ��� �־��� ��Ʈ ���κ��� �ʺ� �켱 Ž�� �������
 * ��� �ڽ� ��带 ��ȸ�ϸ� ���� ������ UI�� �߰��ϴ� ������ �մϴ�.
 *
 * @param RootNode - Ž���� ������ ��Ʈ ���
 */
void UUW_ItemShop::AddNodesBreadthFirst(UUW_ItemListEntry* RootNode)
{
    if (!RootNode)
    {
        return;
    }

    TArray<UUW_ItemListEntry*> NodeQueue;
    NodeQueue.Add(RootNode);

    while (NodeQueue.Num() > 0)
    {
        int32 CurrentLevelSize = NodeQueue.Num();
        UHorizontalBox* CurrentLevelBox = CreateRootHorizontalBox();
        AddTopLevelBoxToItemHierarchy(CurrentLevelBox);

        for (int32 i = 0; i < CurrentLevelSize; ++i)
        {
            UUW_ItemListEntry* CurrentNode = NodeQueue[0];
            NodeQueue.RemoveAt(0);

            if (CurrentNode)
            {
                AddNodeToHorizontalBox(CurrentLevelBox, CurrentNode, ESlateSizeRule::Fill, HAlign_Center, VAlign_Center);

                for (auto& ChildNode : CurrentNode->ChildNodes)
                {
                    NodeQueue.Add(ChildNode);
                }
            }
        }
    }
}


/**
 * AddChildNodesRecursively �Լ��� �־��� �θ� ��带 �������� ���� �켱 Ž�� �������
 * ��� �ڽ� ��带 ��ȸ�ϸ� ���� ������ UI�� ��������� �߰��ϴ� ������ �մϴ�.
 *
 * @param ParentNode - Ž���� ������ �θ� ���
 * @param ParentBox - �θ� ��带 ������ UI �ڽ�
 * @param Depth - ���� ���� ���� (��Ʈ ���� 0)
 */
void UUW_ItemShop::AddChildNodesRecursively(UUW_ItemListEntry* ParentNode, UHorizontalBox* ParentBox, int32 Depth)
{
    AddNodeToHorizontalBox(ParentBox, ParentNode, ESlateSizeRule::Fill, HAlign_Center, VAlign_Center);

    if (ParentNode->ChildNodes.Num() > 0)
    {
        // ���̰� 0�� ��쿡�� �ֻ��� �ڽ��� ItemHierarchyBox�� �߰�
        if (Depth == 0)
        {
            AddTopLevelBoxToItemHierarchy(ParentBox);
        }

        // �ڽ� ��带 ������ �� HorizontalBox ����
        UHorizontalBox* ChildContainerBox = WidgetTree->ConstructWidget<UHorizontalBox>(
            UHorizontalBox::StaticClass(),
            FName(FString::Printf(TEXT("HorizontalBox%d"), BoxIndex++))
        );

        // �� �ڽ� ��带 ChildContainerBox�� �߰�
        for (UUW_ItemListEntry* ChildNode : ParentNode->ChildNodes)
        {
            AddNodeToHorizontalBox(ChildContainerBox, ChildNode, ESlateSizeRule::Fill, HAlign_Center, VAlign_Center);

            // �ڽ� ��带 ��������� �߰�
            AddChildNodesRecursively(ChildNode, ChildContainerBox, Depth + 1);
        }

        // ��� �ڽ� ��带 �߰��� �� ChildContainerBox�� ItemHierarchyBox�� �߰�
        AddTopLevelBoxToItemHierarchy(ChildContainerBox);
    }
}


/**
 * UUW_ItemShop Ŭ������ ���̾ƿ� �� ���� ���� ������ ���� ��ƿ��Ƽ �Լ����Դϴ�.
 *
 * �Լ� ���:
 *
 * - SetupStackBoxSlot: �־��� StackBoxSlot�� ���� ���� �� ũ�� ������ �����մϴ�.
 * - SetupHorizontalBoxSlot: �־��� HorizontalBoxSlot�� ���� ���� �� ũ�� ������ �����մϴ�.
 * - AddNodeToStackBox: StackBox�� ��带 �߰��ϰ�, �ش� ����� ���� �� ũ�⸦ �����մϴ�.
 * - AddTopLevelBoxToItemHierarchy: �ֻ��� �ڽ��� ������ ���� ������ �߰��մϴ�.
 * - AddNodeToHorizontalBox: HorizontalBox�� ��带 �߰��ϰ�, �ش� ����� ���� �� ũ�⸦ �����մϴ�.
 */
void UUW_ItemShop::SetupStackBoxSlot(UStackBoxSlot* NewSlot, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment)
{
    if (NewSlot)
    {
        NewSlot->SetSize(SizeRule);
        NewSlot->SetHorizontalAlignment(HorizontalAlignment);
        NewSlot->SetVerticalAlignment(VerticalAlignment);
    }
}

void UUW_ItemShop::SetupHorizontalBoxSlot(UHorizontalBoxSlot* NewSlot, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment)
{
    if (NewSlot)
    {
        NewSlot->SetSize(SizeRule);
        NewSlot->SetHorizontalAlignment(HorizontalAlignment);
        NewSlot->SetVerticalAlignment(VerticalAlignment);
    }
}

void UUW_ItemShop::AddNodeToStackBox(UStackBox* ParentBox, UWidget* Node, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment)
{
    UStackBoxSlot* StackBoxSlot = ParentBox->AddChildToStackBox(Node);
    if (StackBoxSlot)
    {
        SetupStackBoxSlot(StackBoxSlot, SizeRule, HorizontalAlignment, VerticalAlignment);
    }
}

void UUW_ItemShop::AddTopLevelBoxToItemHierarchy(UHorizontalBox* TopLevelBox)
{
    AddNodeToStackBox(ItemHierarchyBox, TopLevelBox, ESlateSizeRule::Fill, HAlign_Fill, VAlign_Fill);
}

void UUW_ItemShop::AddNodeToHorizontalBox(UHorizontalBox* ParentBox, UWidget* Node, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment)
{
    UHorizontalBoxSlot* HorizontalBoxSlot = ParentBox->AddChildToHorizontalBox(Node);
    if (HorizontalBoxSlot)
    {
        SetupHorizontalBoxSlot(HorizontalBoxSlot, SizeRule, HorizontalAlignment, VerticalAlignment);
    }
}

UHorizontalBox* UUW_ItemShop::CreateRootHorizontalBox()
{
    UHorizontalBox* HorizontalBox = WidgetTree->ConstructWidget<UHorizontalBox>(
        UHorizontalBox::StaticClass(),
        FName(FString::Printf(TEXT("RootHorizontalBox%d"), BoxIndex++))
    );

    UStackBoxSlot* StackBoxSlot = ItemHierarchyBox->AddChildToStackBox(HorizontalBox);
    if (StackBoxSlot)
    {
        SetupStackBoxSlot(StackBoxSlot, ESlateSizeRule::Fill, HAlign_Fill, VAlign_Fill);
    }

    return HorizontalBox;
}

/**
 * FindLoadedItemByID �Լ��� �ε�� ������ ����Ʈ���� �־��� ItemID�� ��ġ�ϴ� �������� ã���ϴ�.
 * ��ġ�ϴ� �������� �ִٸ� �ش� �������� ��ȯ�ϰ�, ���ٸ� nullptr�� ��ȯ�մϴ�.
 *
 * @param ItemID - ã���� �ϴ� �������� ID
 * @return ��ġ�ϴ� UUW_ItemListEntry ��ü �Ǵ� nullptr
 */
UUW_ItemListEntry* UUW_ItemShop::FindLoadedItemByID(int32 ItemID) const
{
    for (UUW_ItemListEntry* ItemEntry : LoadedItems)
    {
        if (ItemEntry && ItemEntry->ItemID == ItemID)
        {
            return ItemEntry;
        }
    }
    return nullptr;
}


/**
 * �־��� ������ ������ ���� ������ ���� ������ �����ϰų�, �̹� �����ϴ� ���� ������ ��ȯ�մϴ�.
 *
 * @param ItemInfo - ������ ����
 * @return UUW_ItemListEntry ��ü (���� ������ ��Ʈ ���)
 */
UUW_ItemListEntry* UUW_ItemShop::GetOrCreateItemHierarchy(const FItemInformation& ItemInfo)
{
    // �ֻ��� �������� ��� ParentNodeID�� 0���� ����
    int32 ParentNodeID = 0;

    if (TArray<TWeakObjectPtr<UUW_ItemListEntry>>* CachedNodes = ItemHierarchyCache.Find(ItemInfo.ItemID))
    {
        for (TWeakObjectPtr<UUW_ItemListEntry>& CachedNode : *CachedNodes)
        {
            if (CachedNode.IsValid())
            {
                UE_LOG(LogTemp, Log, TEXT("[UUW_ItemShop::GetOrCreateItemHierarchy] Using cached item hierarchy for ItemID: %d"), ItemInfo.ItemID);
                return CachedNode.Get();
            }
        }
    }

    UUW_ItemListEntry* NewHierarchy = CreateNodeHierarchy(ItemInfo);
    if (NewHierarchy)
    {
        if (!ItemHierarchyCache.Contains(ItemInfo.ItemID))
        {
            ItemHierarchyCache.Add(ItemInfo.ItemID, TArray<TWeakObjectPtr<UUW_ItemListEntry>>());
        }

        ItemHierarchyCache[ItemInfo.ItemID].Add(NewHierarchy);
        UE_LOG(LogTemp, Log, TEXT("[UUW_ItemShop::GetOrCreateItemHierarchy] Created new item hierarchy for ItemID: %d"), ItemInfo.ItemID);
    }
    return NewHierarchy;
}


/**
 * �־��� ��� ������ ���� ��带 �����մϴ�.
 *
 * @param NodeInfo - ��� ����
 * @return UUW_ItemListEntry ��ü (ĳ�õ� ���)
 */
UUW_ItemListEntry* UUW_ItemShop::CreateCachedNode(const FItemInformation& NodeInfo)
{
    UUW_ItemListEntry* NewNode = CreateWidget<UUW_ItemListEntry>(this, ItemListEntryClass);
    if (NewNode)
    {
        NewNode->SetupNode(NodeInfo.ItemID, NodeInfo.Icon, FString::Printf(TEXT("%d"), NodeInfo.Price));
        NewNode->BindItemShopWidget(this);
        NewNode->UpdateDisplaySubItems(false);

        // �� ���� ĳ������ �ʽ��ϴ�.
        if (NodeInfo.ItemID != 0)
        {
            if (!ItemHierarchyCache.Contains(NodeInfo.ItemID))
            {
                ItemHierarchyCache.Add(NodeInfo.ItemID, TArray<TWeakObjectPtr<UUW_ItemListEntry>>());
            }

            ItemHierarchyCache[NodeInfo.ItemID].Add(NewNode);
        }
    }
    return NewNode;
}


void UUW_ItemShop::SetSelectedItem(UUW_ItemListEntry* NewSelectedItem)
{
    if (SelectedItem.IsValid())
    {
        // Deactivate the background of the previously selected item
        SelectedItem->SetItemSelected(false);
    }

    // Select the new item and activate its background
    SelectedItem = NewSelectedItem;
    if (SelectedItem.IsValid())
    {
        SelectedItem->SetItemSelected(true);
    }
}

void UUW_ItemShop::PlaySound(USoundBase* Sound)
{
    // ���� �ε� ���� ���� Ȯ�� �� �α� ���
    if (Sound)
    {
        // ���� ���
        UGameplayStatics::PlaySound2D(GetWorld(), Sound, 1.0f, 1.0f, 0.0f);
        UE_LOG(LogTemp, Log, TEXT("Playing sound: %s"), *Sound->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load sound: %s"), *Sound->GetName());
    }
}

FItemInformation* UUW_ItemShop::GetItemInfoByID(int32 ItemID)
{
    return GameState.IsValid() ? GameState->GetItemInfoByID(ItemID) : nullptr;
}
