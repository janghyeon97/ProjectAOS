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
 * AddClassifiedItemEntry 함수는 주어진 아이템 정보를 기반으로 분류된 아이템 항목을 UI에 추가합니다.
 * 아이템 목록에 항목을 추가하고, 적절한 분류 패널에 항목을 배치합니다.
 *
 * @param FItemInformation - 아이템 정보 구조체
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
 * GetOrCreateClassificationPanel 함수는 주어진 분류 문자열에 해당하는 UUniformGridPanel을 반환합니다.
 * 패널이 이미 존재하는 경우 해당 패널을 반환하고, 존재하지 않는 경우 새로 생성하여 반환합니다.
 *
 * @param ClassificationString - 분류 문자열
 * @return UUniformGridPanel 객체 (분류 패널)
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

    // 아이템 구매 후 UI 업데이트
    if (SelectedItem.IsValid())
    {
        DisplayItemWithSubItems(SelectedItem->ItemID);
    }
   
    // 구매 완료 사운드 재생
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
 * DisplayItemDescription 함수는 주어진 아이템 인덱스를 기반으로 아이템의 설명을 UI에 표시합니다.
 * 캐릭터와 스탯 컴포넌트를 가져와 아이템 정보를 기반으로 설명을 추가합니다.
 *
 * @param ItemIndex - 아이템 인덱스
 */
void UUW_ItemShop::DisplayItemDescription(int32 ItemIndex)
{
    // 캐릭터 가져오기
    AAOSCharacterBase* Character = Cast<AAOSCharacterBase>(OwningActor);
    if (!::IsValid(Character))
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::DisplaySubItems] Invalid Character"));
        return;
    }

    // 스탯 컴포넌트 가져오기
    UStatComponent* StatComponent = Character->GetStatComponent();
    if (!::IsValid(StatComponent))
    {
        UE_LOG(LogTemp, Error, TEXT("[UUW_ItemShop::DisplaySubItems] Invalid StatComponent"));
        return;
    }

    // 아이템 정보 가져오기
    FItemInformation* SubItemInfo = GameState->GetItemInfoByID(ItemIndex);
    if (!SubItemInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ItemShop::DisplayItemDescription] Could not find SubItemInfo for ItemID: %d"), ItemIndex);
        return;
    }

    AddItemDescription(*SubItemInfo, StatComponent);

    // 아이템 정보 업데이트
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
 * AddItemDescription 함수는 주어진 아이템 정보와 스탯 컴포넌트를 사용하여 아이템의 능력 및 설명을 UI에 추가합니다.
 * 기존 설명을 지우고 새로운 설명을 추가한 후 캐시에 저장합니다.
 *
 * @param ItemInfo - 아이템 정보
 * @param StatComponent - 스탯 컴포넌트
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
 * CreateNodeHierarchy 함수는 주어진 아이템 정보(NodeInfo)에 따라 재귀적으로 노드 계층 구조를 생성합니다.
 * 각 노드는 자식 노드를 포함하며, 모든 노드는 캐시를 통해 관리됩니다.
 *
 * @param NodeInfo - 아이템 정보
 * @return UUW_ItemListEntry 객체 (계층 구조의 루트 노드)
 */
UUW_ItemListEntry* UUW_ItemShop::CreateNodeHierarchy(const FItemInformation& NodeInfo)
{
    // 캐시에서 노드를 찾거나 새로 생성하여 반환
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

    // 빈 노드 추가
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

    // 빈 노드 추가해서 균형 맞추기
    while (RootNode->ChildNodes.Num() < MaxChildCount)
    {
        UUW_ItemListEntry* EmptyNode = CreateCachedNode(FItemInformation());
        EmptyNode->ParentNode = RootNode;
        RootNode->ChildNodes.Add(EmptyNode);
    }

    return RootNode;
}

/**
 * AddNodesBreadthFirst 함수는 주어진 루트 노드로부터 너비 우선 탐색 방식으로
 * 모든 자식 노드를 순회하며 계층 구조를 UI에 추가하는 역할을 합니다.
 *
 * @param RootNode - 탐색을 시작할 루트 노드
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
 * AddChildNodesRecursively 함수는 주어진 부모 노드를 기준으로 깊이 우선 탐색 방식으로
 * 모든 자식 노드를 순회하며 계층 구조를 UI에 재귀적으로 추가하는 역할을 합니다.
 *
 * @param ParentNode - 탐색을 시작할 부모 노드
 * @param ParentBox - 부모 노드를 포함할 UI 박스
 * @param Depth - 현재 깊이 수준 (루트 노드는 0)
 */
void UUW_ItemShop::AddChildNodesRecursively(UUW_ItemListEntry* ParentNode, UHorizontalBox* ParentBox, int32 Depth)
{
    AddNodeToHorizontalBox(ParentBox, ParentNode, ESlateSizeRule::Fill, HAlign_Center, VAlign_Center);

    if (ParentNode->ChildNodes.Num() > 0)
    {
        // 깊이가 0인 경우에만 최상위 박스를 ItemHierarchyBox에 추가
        if (Depth == 0)
        {
            AddTopLevelBoxToItemHierarchy(ParentBox);
        }

        // 자식 노드를 포함할 새 HorizontalBox 생성
        UHorizontalBox* ChildContainerBox = WidgetTree->ConstructWidget<UHorizontalBox>(
            UHorizontalBox::StaticClass(),
            FName(FString::Printf(TEXT("HorizontalBox%d"), BoxIndex++))
        );

        // 각 자식 노드를 ChildContainerBox에 추가
        for (UUW_ItemListEntry* ChildNode : ParentNode->ChildNodes)
        {
            AddNodeToHorizontalBox(ChildContainerBox, ChildNode, ESlateSizeRule::Fill, HAlign_Center, VAlign_Center);

            // 자식 노드를 재귀적으로 추가
            AddChildNodesRecursively(ChildNode, ChildContainerBox, Depth + 1);
        }

        // 모든 자식 노드를 추가한 후 ChildContainerBox를 ItemHierarchyBox에 추가
        AddTopLevelBoxToItemHierarchy(ChildContainerBox);
    }
}


/**
 * UUW_ItemShop 클래스의 레이아웃 및 계층 구조 설정을 위한 유틸리티 함수들입니다.
 *
 * 함수 목록:
 *
 * - SetupStackBoxSlot: 주어진 StackBoxSlot에 대한 정렬 및 크기 설정을 구성합니다.
 * - SetupHorizontalBoxSlot: 주어진 HorizontalBoxSlot에 대한 정렬 및 크기 설정을 구성합니다.
 * - AddNodeToStackBox: StackBox에 노드를 추가하고, 해당 노드의 정렬 및 크기를 설정합니다.
 * - AddTopLevelBoxToItemHierarchy: 최상위 박스를 아이템 계층 구조에 추가합니다.
 * - AddNodeToHorizontalBox: HorizontalBox에 노드를 추가하고, 해당 노드의 정렬 및 크기를 설정합니다.
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
 * FindLoadedItemByID 함수는 로드된 아이템 리스트에서 주어진 ItemID와 일치하는 아이템을 찾습니다.
 * 일치하는 아이템이 있다면 해당 아이템을 반환하고, 없다면 nullptr을 반환합니다.
 *
 * @param ItemID - 찾고자 하는 아이템의 ID
 * @return 일치하는 UUW_ItemListEntry 객체 또는 nullptr
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
 * 주어진 아이템 정보에 대한 아이템 계층 구조를 생성하거나, 이미 존재하는 계층 구조를 반환합니다.
 *
 * @param ItemInfo - 아이템 정보
 * @return UUW_ItemListEntry 객체 (계층 구조의 루트 노드)
 */
UUW_ItemListEntry* UUW_ItemShop::GetOrCreateItemHierarchy(const FItemInformation& ItemInfo)
{
    // 최상위 아이템의 경우 ParentNodeID는 0으로 설정
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
 * 주어진 노드 정보에 대한 노드를 생성합니다.
 *
 * @param NodeInfo - 노드 정보
 * @return UUW_ItemListEntry 객체 (캐시된 노드)
 */
UUW_ItemListEntry* UUW_ItemShop::CreateCachedNode(const FItemInformation& NodeInfo)
{
    UUW_ItemListEntry* NewNode = CreateWidget<UUW_ItemListEntry>(this, ItemListEntryClass);
    if (NewNode)
    {
        NewNode->SetupNode(NodeInfo.ItemID, NodeInfo.Icon, FString::Printf(TEXT("%d"), NodeInfo.Price));
        NewNode->BindItemShopWidget(this);
        NewNode->UpdateDisplaySubItems(false);

        // 빈 노드는 캐시하지 않습니다.
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
    // 사운드 로드 성공 여부 확인 및 로그 출력
    if (Sound)
    {
        // 사운드 재생
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
