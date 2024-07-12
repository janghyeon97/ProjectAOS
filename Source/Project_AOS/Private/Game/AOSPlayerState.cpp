#include "Game/AOSPlayerState.h"
#include "Game/AOSGameInstance.h"
#include "Game/AOSGameMode.h"
#include "Game/LobbyGameState.h"
#include "Game/PlayerStateSave.h"
#include "Controllers/AOSPlayerController.h"
#include "Characters/AOSCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Item/ItemData.h"
#include "Item/Item.h"

constexpr int32 MaxInventorySize = 6;

AAOSPlayerState::AAOSPlayerState()
{
    bReplicates = true;

    TeamSide = ETeamSideBase::Type;
    PlayerIndex = -1;
    SelectedChampionIndex = -1;
    AbilityPoints = 0;
    Currency = 10000;
    PlayerUniqueID = FString();

    Inventory.SetNum(MaxInventorySize);
}

void AAOSPlayerState::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner())
    {
        UE_LOG(LogTemp, Log, TEXT("PlayerState's owner is: %s (Class: %s)"), *GetOwner()->GetName(), *GetOwner()->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerState's owner is not set"));
    }
}

void AAOSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, TeamSide);
    DOREPLIFETIME(ThisClass, PlayerIndex);
    DOREPLIFETIME(ThisClass, SelectedChampionIndex);
    DOREPLIFETIME(ThisClass, AbilityPoints);
    DOREPLIFETIME(ThisClass, Currency);
}

// Accessors

FString AAOSPlayerState::GetPlayerUniqueID() const
{
    if (GetUniqueId().IsValid() && GetUniqueId()->IsValid())
    {
        return GetUniqueId()->ToString();
    }
    return FString();
}

// Inventory Management

void AAOSPlayerState::AddItemToInventory(AItem* Item)
{
    if (Item->CurrentStack <= 0)
    {
        Item->CurrentStack = 1;
    }

    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (Inventory[i] && Inventory[i]->ItemID == Item->ItemID)
        {
            if (Inventory[i]->CurrentStack < Inventory[i]->MaxStack)
            {
                Inventory[i]->CurrentStack++;
                SendLoadedItemsToClients();
                return;
            }
        }
    }

    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (!Inventory[i])
        {
            Inventory[i] = Item;
            BindItem(Item);
            SendLoadedItemsToClients();
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Inventory is full"));
}

void AAOSPlayerState::RemoveItemFromInventory(int32 ItemID)
{
    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (Inventory[i] && Inventory[i]->ItemID == ItemID)
        {
            UE_LOG(LogTemp, Log, TEXT("Removing item: %d from inventory slot %d"), ItemID, i);
            Inventory[i]->RemoveAbilitiesFromCharacter();
            Inventory[i] = nullptr;
            SendLoadedItemsToClients();
            break;
        }
    }
}

void AAOSPlayerState::SwapItemsInInventory(int32 Index1, int32 Index2)
{
    if (Inventory.IsValidIndex(Index1) && Inventory.IsValidIndex(Index2))
    {
        Inventory.Swap(Index1, Index2);
        SendLoadedItemsToClients();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Invalid inventory indices"));
    }
}

void AAOSPlayerState::UseItem(int32 Index)
{
    if (Inventory.IsValidIndex(Index) && Inventory[Index])
    {
        Inventory[Index]->Use(this);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AAOSPlayerState::UseItem] Invalid item index or item not found"));
    }
}

bool AAOSPlayerState::IsInventoryFull() const
{
    for (const auto& InvItem : Inventory)
    {
        if (!InvItem || InvItem->CurrentStack < InvItem->MaxStack)
        {
            return false;
        }
    }
    return true;
}

int32 AAOSPlayerState::GetTotalItemCount(int32 ItemID) const
{
    int32 TotalCount = 0;
    for (const auto& InvItem : Inventory)
    {
        if (InvItem && InvItem->ItemID == ItemID)
        {
            TotalCount += InvItem->CurrentStack;
        }
    }
    return TotalCount;
}

// Server Functions

bool AAOSPlayerState::ServerPurchaseItem_Validate(int32 ItemID)
{
    return true;
}

/**
 * 플레이어가 아이템을 구매할 때 서버 측에서 처리하는 함수입니다.
 *
 * 이 함수는 플레이어가 아이템을 구매하려 할 때 호출됩니다.
 * 여러 가지 확인을 수행합니다:
 * - 게임 모드의 로드된 아이템 목록에서 해당 아이템이 존재하는지 확인합니다.
 * - 플레이어의 인벤토리에 아이템 객체를 복제합니다.
 * - 적용 가능한 할인 항목을 적용하고, 필요한 하위 아이템을 플레이어 인벤토리에서 제거합니다.
 * - 플레이어가 아이템을 구매할 충분한 자금을 가지고 있는지 확인합니다.
 * - 플레이어의 인벤토리가 가득 차 있지 않은지, 플레이어가 해당 아이템의 최대 소지 한도를 초과하지 않았는지 확인합니다.
 *
 * 모든 확인을 통과하면 아이템이 플레이어의 인벤토리에 추가되고, 플레이어의 자금이 아이템의 최종 가격만큼 감소합니다.
 * 함수는 디버깅 목적으로 구매 시도의 결과를 로그에 기록합니다.
 *
 * @param ItemID 플레이어가 구매하려는 아이템의 ID입니다.
 */
void AAOSPlayerState::ServerPurchaseItem_Implementation(int32 ItemID)
{
    AAOSGameMode* GameMode = Cast<AAOSGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode)
    {
        AItem** FoundItem = GameMode->LoadedItems.Find(ItemID);
        if (FoundItem && *FoundItem)
        {
            int32 CurrentItemCount = GetTotalItemCount(ItemID);

            // 최대 소지 한도를 초과하지 않았는지 확인합니다.
            if (CurrentItemCount >= (*FoundItem)->MaxPossessQuantity)
            {
                UE_LOG(LogTemp, Log, TEXT("Cannot purchase item: %s. Maximum possession limit reached"), *(*FoundItem)->Name);
                return;
            }

            AItem* Item = DuplicateObject<AItem>(*FoundItem, this);
            Item->CurrentStack = 1;
            int32 FinalPrice = 0;

            // 할인을 적용하고 필요한 하위 아이템을 제거합니다.
            ApplyDiscountAndRemoveSubItems(Item, FinalPrice);

            if (Currency < FinalPrice)
            {
                UE_LOG(LogTemp, Log, TEXT("Insufficient funds"));
                return;
            }

            // 인벤토리가 가득 찼는지와 상위 아이템을 추가할 수 있는지 확인합니다.
            if (IsInventoryFull() && !CanAddItemWithSubItems(Item))
            {
                UE_LOG(LogTemp, Log, TEXT("Cannot purchase item: %s. Inventory is full"), *Item->Name);
                return;
            }

            // 아이템을 인벤토리에 추가하고 자금을 차감합니다.
            AddItemToInventory(Item);
            Currency -= FinalPrice;

            ClientOnItemPurchased(Item->ItemID);

            UE_LOG(LogTemp, Log, TEXT("Purchase successful: %s"), *Item->Name);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Invalid item ID"));
        }
    }
}

// Helper Functions ---------------------------------------------------------------------

/**
 * 주어진 아이템을 추가할 때 필요한 하위 아이템이 인벤토리에 있는지 확인하고,
 * 모든 하위 아이템이 존재하면 이를 제거한 후 아이템을 추가할 수 있는지 여부를 반환합니다.
 *
 * @param Item 추가하려는 대상 아이템.
 * @return 필요한 모든 하위 아이템이 인벤토리에 존재하고 제거되면 true, 그렇지 않으면 false.
 */
bool AAOSPlayerState::CanAddItemWithSubItems(AItem* Item)
{
    TArray<int32> ItemsToRemove;

    // Item 객체의 RequiredItems 배열을 순회하여 필요한 하위 아이템을 확인합니다.
    for (const int32& RequiredItemID : Item->RequiredItems)
    {
        bool bItemFound = false;

        for (AItem* InvItem : Inventory)
        {
            if (InvItem && InvItem->ItemID == RequiredItemID)
            {
                ItemsToRemove.Add(RequiredItemID);
                bItemFound = true;
                break;
            }
        }

        // 필요한 하위 아이템이 하나라도 인벤토리에 없는 경우 false를 반환합니다.
        if (!bItemFound)
        {
            return false;
        }
    }

    // 모든 하위 아이템을 인벤토리에서 제거합니다.
    for (int32 ItemIDToRemove : ItemsToRemove)
    {
        RemoveItemFromInventory(ItemIDToRemove);
    }

    return true;
}



/**
 * 아이템에 필요한 하위 아이템의 가격을 할인하여 최종 가격에 반영하고,
 * 필요한 하위 아이템을 인벤토리에서 제거합니다.
 *
 * 이 함수는 주어진 아이템의 `RequiredItems` 배열을 순회하면서
 * 인벤토리에서 해당 하위 아이템을 찾습니다.
 * 하위 아이템의 가격을 최종 가격에서 차감하고, 해당 하위 아이템을 인벤토리에서 제거합니다.
 * 필요한 모든 하위 아이템이 처리될 때까지 재귀적으로 호출됩니다.
 *
 * @param Item 가격에 할인을 적용할 대상 아이템.
 * @param OutFinalPrice 할인이 적용된 최종 가격을 저장할 변수.
 */
void AAOSPlayerState::ApplyDiscountAndRemoveSubItems(AItem* Item, int32& OutFinalPrice)
{
    OutFinalPrice = Item->Price;
    ApplyDiscount(Item, OutFinalPrice);
}

void AAOSPlayerState::ApplyDiscount(AItem* Item, int32& OutFinalPrice)
{
    for (int32 RequiredItemID : Item->RequiredItems)
    {
        for (AItem* OwnedItem : Inventory)
        {
            if (::IsValid(OwnedItem) && OwnedItem->ItemID == RequiredItemID)
            {
                OutFinalPrice = FMath::Max(0, OutFinalPrice - OwnedItem->Price);
                RemoveItemFromInventory(RequiredItemID);
                ApplyDiscount(OwnedItem, OutFinalPrice);
                break;
            }
        }
    }
}


void AAOSPlayerState::BindItem(AItem* Item)
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerState's owner is not set"));
        return;
    }

    // PlayerController로 캐스팅
    APlayerController* PlayerController = Cast<APlayerController>(OwnerActor);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Owner is not a PlayerController"));
        return;
    }

    // PlayerController의 Pawn을 가져와서 AAOSCharacterBase로 캐스팅
    APawn* ControlledPawn = PlayerController->GetPawn();
    AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(ControlledPawn);

    if (!IsValid(OwningCharacter))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to bind item %s: Controlled character is not valid or is not AAOSCharacterBase"), *Item->GetName());
        return;
    }

    // 아이템을 캐릭터에 바인딩
    UE_LOG(LogTemp, Log, TEXT("Binding item %s to character %s"), *Item->GetName(), *OwningCharacter->GetName());
    Item->BindToPlayer(OwningCharacter);
}
   

/** --------------------------------------------------------------------------------
 * 아이템 타이머 관리 함수들
 *
 * 이 함수들은 플레이어 상태에서 아이템의 타이머를 관리하는 데 사용됩니다.
 * 타이머가 활성화되어 있는지 확인하고, 타이머를 설정하거나 제거하며,
 * 타이머의 남은 시간을 주기적으로 브로드캐스트하는 기능을 제공합니다.
 */


/**
  * 아이템 타이머가 활성화되어 있는지 확인합니다.
  *
  * 주어진 아이템 ID에 해당하는 타이머가 활성화되어 있는지 확인합니다.
  *
  * @param ItemID 확인할 아이템의 ID.
  * @return 타이머가 활성화되어 있으면 true, 그렇지 않으면 false.
  */
bool AAOSPlayerState::IsItemTimerActive(const int32 ItemID) const
{
    const FTimerHandle* TimerHandle = ItemTimerHandles.Find(ItemID);
    return TimerHandle && GetWorld()->GetTimerManager().IsTimerActive(*TimerHandle);
}


/**
 * 아이템 타이머를 설정합니다.
 *
 * 주어진 아이템 ID에 대해 타이머를 설정하고, 타이머가 만료되었을 때 실행할 콜백을 지정합니다.
 * 또한, 주기적으로 남은 시간을 브로드캐스트하는 보조 타이머를 설정합니다.
 *
 * @param ItemID 타이머를 설정할 아이템의 ID.
 * @param Duration 타이머의 지속 시간.
 * @param Callback 타이머가 만료되었을 때 실행할 콜백 함수.
 */
void AAOSPlayerState::SetItemTimer(int32 ItemID, TFunction<void()> Callback, float Duration, bool bLoop, float FirstDelay)
{
    FTimerHandle& TimerHandle = ItemTimerHandles.FindOrAdd(ItemID);
    FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(Callback);
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, bLoop, FirstDelay);

    // 주기적으로 남은 시간을 브로드캐스트하는 타이머 설정
    FTimerHandle& BroadcastTimerHandle = BroadcastTimerHandles.FindOrAdd(ItemID);
    GetWorld()->GetTimerManager().SetTimer(BroadcastTimerHandle, [this, ItemID]()
        {
            float RemainingTime = GetItemTimerRemaining(ItemID);
            BroadcastRemainingTime(ItemID, RemainingTime);
        }, 1.0f, true, 0.0f);
}


/**
 * 아이템 타이머를 제거합니다.
 *
 * 주어진 아이템 ID에 대해 활성화된 타이머와 주기적으로 남은 시간을 브로드캐스트하는 타이머를 제거합니다.
 *
 * @param ItemID 제거할 타이머의 아이템 ID.
 */
void AAOSPlayerState::ClearItemTimer(const int32 ItemID)
{
    FTimerHandle* TimerHandle = ItemTimerHandles.Find(ItemID);
    if (TimerHandle)
    {
        GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
        ItemTimerHandles.Remove(ItemID);
    }

    // 브로드캐스트 타이머도 제거
    FTimerHandle* BroadcastTimerHandle = BroadcastTimerHandles.Find(ItemID);
    if (BroadcastTimerHandle)
    {
        GetWorld()->GetTimerManager().ClearTimer(*BroadcastTimerHandle);
        BroadcastTimerHandles.Remove(ItemID);
    }
}


/**
 * 아이템 타이머의 남은 시간을 반환합니다.
 *
 * 주어진 아이템 ID에 대해 타이머의 남은 시간을 반환합니다.
 *
 * @param ItemID 남은 시간을 확인할 타이머의 아이템 ID.
 * @return 타이머의 남은 시간(초) 또는 타이머가 없으면 0.
 */
float AAOSPlayerState::GetItemTimerRemaining(const int32 ItemID) const
{
    const FTimerHandle* TimerHandle = ItemTimerHandles.Find(ItemID);
    return TimerHandle ? GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandle) : 0.f;
}

void AAOSPlayerState::BroadcastRemainingTime_Implementation(int32 ItemID, float RemainingTime)
{
    if (OnItemTimerUpdated.IsBound())
    {
        OnItemTimerUpdated.Broadcast(ItemID, RemainingTime);
    }
}

// Events

void AAOSPlayerState::UpdateSelectedChampion_Server_Implementation(int32 Index)
{
    SelectedChampionIndex = Index;
}

void AAOSPlayerState::OnPlayerLevelChanged(int32 OldLevel, int32 NewLevel)
{
    AbilityPoints = AbilityPoints + 1;
}

void AAOSPlayerState::ConvertInventoryItems(TArray<FItemInformation>& OutItems) const
{
    OutItems.Reserve(Inventory.Num());

    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (Inventory[i] != nullptr)
        {
            FItemInformation ItemData;
            ItemData.ItemID = Inventory[i]->ItemID;
            ItemData.Name = Inventory[i]->Name;
            ItemData.Price = Inventory[i]->Price;
            ItemData.Description = Inventory[i]->Description;
            ItemData.Icon = Inventory[i]->Icon;
            ItemData.ItemStackLimit = Inventory[i]->MaxStack;
            ItemData.CurrentStack = Inventory[i]->CurrentStack;
            ItemData.Classification = Inventory[i]->Classification;
            ItemData.Abilities = Inventory[i]->Abilities;

            OutItems.Add(ItemData);

#if UE_BUILD_DEBUG
            UE_LOG(LogTemp, Log, TEXT("Added Index %d item to OutItems: ID=%d, Name=%s, Price=%d, Description=%s"),
                i,
                ItemData.ItemID,
                *ItemData.Name,
                ItemData.Price,
                *ItemData.Description);
#endif
        }
        else
        {
            OutItems.Add(FItemInformation());
        }
    }
}

void AAOSPlayerState::SendLoadedItemsToClients()
{
    if (HasAuthority())
    {
        TArray<FItemInformation> OutItems;
        ConvertInventoryItems(OutItems);
        ClientInventoryChanged(OutItems);
    }
}

void AAOSPlayerState::OnRep_CurrencyUpdated()
{
    if (OnCurrencyUpdated.IsBound())
    {
        OnCurrencyUpdated.Broadcast(Currency);
    }
}

void AAOSPlayerState::ClientInventoryChanged_Implementation(const TArray<FItemInformation>& Items)
{
    if (OnInventoryUpdated.IsBound())
    {
        OnInventoryUpdated.Broadcast(Items);
    }
}

void AAOSPlayerState::ClientOnItemPurchased_Implementation(int32 ItemID)
{
    if (OnItemPurchased.IsBound())
    {
        UE_LOG(LogTemp, Log, TEXT("[AAOSPlayerState::ClientOnItemPurchased] Broadcasting OnItemPurchased for ItemID: %d"), ItemID);
        OnItemPurchased.Broadcast(ItemID);
    }
}


void AAOSPlayerState::SetOwner(AActor* NewOwner)
{
    Super::SetOwner(NewOwner);


}