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
 * �÷��̾ �������� ������ �� ���� ������ ó���ϴ� �Լ��Դϴ�.
 *
 * �� �Լ��� �÷��̾ �������� �����Ϸ� �� �� ȣ��˴ϴ�.
 * ���� ���� Ȯ���� �����մϴ�:
 * - ���� ����� �ε�� ������ ��Ͽ��� �ش� �������� �����ϴ��� Ȯ���մϴ�.
 * - �÷��̾��� �κ��丮�� ������ ��ü�� �����մϴ�.
 * - ���� ������ ���� �׸��� �����ϰ�, �ʿ��� ���� �������� �÷��̾� �κ��丮���� �����մϴ�.
 * - �÷��̾ �������� ������ ����� �ڱ��� ������ �ִ��� Ȯ���մϴ�.
 * - �÷��̾��� �κ��丮�� ���� �� ���� ������, �÷��̾ �ش� �������� �ִ� ���� �ѵ��� �ʰ����� �ʾҴ��� Ȯ���մϴ�.
 *
 * ��� Ȯ���� ����ϸ� �������� �÷��̾��� �κ��丮�� �߰��ǰ�, �÷��̾��� �ڱ��� �������� ���� ���ݸ�ŭ �����մϴ�.
 * �Լ��� ����� �������� ���� �õ��� ����� �α׿� ����մϴ�.
 *
 * @param ItemID �÷��̾ �����Ϸ��� �������� ID�Դϴ�.
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

            // �ִ� ���� �ѵ��� �ʰ����� �ʾҴ��� Ȯ���մϴ�.
            if (CurrentItemCount >= (*FoundItem)->MaxPossessQuantity)
            {
                UE_LOG(LogTemp, Log, TEXT("Cannot purchase item: %s. Maximum possession limit reached"), *(*FoundItem)->Name);
                return;
            }

            AItem* Item = DuplicateObject<AItem>(*FoundItem, this);
            Item->CurrentStack = 1;
            int32 FinalPrice = 0;

            // ������ �����ϰ� �ʿ��� ���� �������� �����մϴ�.
            ApplyDiscountAndRemoveSubItems(Item, FinalPrice);

            if (Currency < FinalPrice)
            {
                UE_LOG(LogTemp, Log, TEXT("Insufficient funds"));
                return;
            }

            // �κ��丮�� ���� á������ ���� �������� �߰��� �� �ִ��� Ȯ���մϴ�.
            if (IsInventoryFull() && !CanAddItemWithSubItems(Item))
            {
                UE_LOG(LogTemp, Log, TEXT("Cannot purchase item: %s. Inventory is full"), *Item->Name);
                return;
            }

            // �������� �κ��丮�� �߰��ϰ� �ڱ��� �����մϴ�.
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
 * �־��� �������� �߰��� �� �ʿ��� ���� �������� �κ��丮�� �ִ��� Ȯ���ϰ�,
 * ��� ���� �������� �����ϸ� �̸� ������ �� �������� �߰��� �� �ִ��� ���θ� ��ȯ�մϴ�.
 *
 * @param Item �߰��Ϸ��� ��� ������.
 * @return �ʿ��� ��� ���� �������� �κ��丮�� �����ϰ� ���ŵǸ� true, �׷��� ������ false.
 */
bool AAOSPlayerState::CanAddItemWithSubItems(AItem* Item)
{
    TArray<int32> ItemsToRemove;

    // Item ��ü�� RequiredItems �迭�� ��ȸ�Ͽ� �ʿ��� ���� �������� Ȯ���մϴ�.
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

        // �ʿ��� ���� �������� �ϳ��� �κ��丮�� ���� ��� false�� ��ȯ�մϴ�.
        if (!bItemFound)
        {
            return false;
        }
    }

    // ��� ���� �������� �κ��丮���� �����մϴ�.
    for (int32 ItemIDToRemove : ItemsToRemove)
    {
        RemoveItemFromInventory(ItemIDToRemove);
    }

    return true;
}



/**
 * �����ۿ� �ʿ��� ���� �������� ������ �����Ͽ� ���� ���ݿ� �ݿ��ϰ�,
 * �ʿ��� ���� �������� �κ��丮���� �����մϴ�.
 *
 * �� �Լ��� �־��� �������� `RequiredItems` �迭�� ��ȸ�ϸ鼭
 * �κ��丮���� �ش� ���� �������� ã���ϴ�.
 * ���� �������� ������ ���� ���ݿ��� �����ϰ�, �ش� ���� �������� �κ��丮���� �����մϴ�.
 * �ʿ��� ��� ���� �������� ó���� ������ ��������� ȣ��˴ϴ�.
 *
 * @param Item ���ݿ� ������ ������ ��� ������.
 * @param OutFinalPrice ������ ����� ���� ������ ������ ����.
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

    // PlayerController�� ĳ����
    APlayerController* PlayerController = Cast<APlayerController>(OwnerActor);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Owner is not a PlayerController"));
        return;
    }

    // PlayerController�� Pawn�� �����ͼ� AAOSCharacterBase�� ĳ����
    APawn* ControlledPawn = PlayerController->GetPawn();
    AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(ControlledPawn);

    if (!IsValid(OwningCharacter))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to bind item %s: Controlled character is not valid or is not AAOSCharacterBase"), *Item->GetName());
        return;
    }

    // �������� ĳ���Ϳ� ���ε�
    UE_LOG(LogTemp, Log, TEXT("Binding item %s to character %s"), *Item->GetName(), *OwningCharacter->GetName());
    Item->BindToPlayer(OwningCharacter);
}
   

/** --------------------------------------------------------------------------------
 * ������ Ÿ�̸� ���� �Լ���
 *
 * �� �Լ����� �÷��̾� ���¿��� �������� Ÿ�̸Ӹ� �����ϴ� �� ���˴ϴ�.
 * Ÿ�̸Ӱ� Ȱ��ȭ�Ǿ� �ִ��� Ȯ���ϰ�, Ÿ�̸Ӹ� �����ϰų� �����ϸ�,
 * Ÿ�̸��� ���� �ð��� �ֱ������� ��ε�ĳ��Ʈ�ϴ� ����� �����մϴ�.
 */


/**
  * ������ Ÿ�̸Ӱ� Ȱ��ȭ�Ǿ� �ִ��� Ȯ���մϴ�.
  *
  * �־��� ������ ID�� �ش��ϴ� Ÿ�̸Ӱ� Ȱ��ȭ�Ǿ� �ִ��� Ȯ���մϴ�.
  *
  * @param ItemID Ȯ���� �������� ID.
  * @return Ÿ�̸Ӱ� Ȱ��ȭ�Ǿ� ������ true, �׷��� ������ false.
  */
bool AAOSPlayerState::IsItemTimerActive(const int32 ItemID) const
{
    const FTimerHandle* TimerHandle = ItemTimerHandles.Find(ItemID);
    return TimerHandle && GetWorld()->GetTimerManager().IsTimerActive(*TimerHandle);
}


/**
 * ������ Ÿ�̸Ӹ� �����մϴ�.
 *
 * �־��� ������ ID�� ���� Ÿ�̸Ӹ� �����ϰ�, Ÿ�̸Ӱ� ����Ǿ��� �� ������ �ݹ��� �����մϴ�.
 * ����, �ֱ������� ���� �ð��� ��ε�ĳ��Ʈ�ϴ� ���� Ÿ�̸Ӹ� �����մϴ�.
 *
 * @param ItemID Ÿ�̸Ӹ� ������ �������� ID.
 * @param Duration Ÿ�̸��� ���� �ð�.
 * @param Callback Ÿ�̸Ӱ� ����Ǿ��� �� ������ �ݹ� �Լ�.
 */
void AAOSPlayerState::SetItemTimer(int32 ItemID, TFunction<void()> Callback, float Duration, bool bLoop, float FirstDelay)
{
    FTimerHandle& TimerHandle = ItemTimerHandles.FindOrAdd(ItemID);
    FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(Callback);
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, bLoop, FirstDelay);

    // �ֱ������� ���� �ð��� ��ε�ĳ��Ʈ�ϴ� Ÿ�̸� ����
    FTimerHandle& BroadcastTimerHandle = BroadcastTimerHandles.FindOrAdd(ItemID);
    GetWorld()->GetTimerManager().SetTimer(BroadcastTimerHandle, [this, ItemID]()
        {
            float RemainingTime = GetItemTimerRemaining(ItemID);
            BroadcastRemainingTime(ItemID, RemainingTime);
        }, 1.0f, true, 0.0f);
}


/**
 * ������ Ÿ�̸Ӹ� �����մϴ�.
 *
 * �־��� ������ ID�� ���� Ȱ��ȭ�� Ÿ�̸ӿ� �ֱ������� ���� �ð��� ��ε�ĳ��Ʈ�ϴ� Ÿ�̸Ӹ� �����մϴ�.
 *
 * @param ItemID ������ Ÿ�̸��� ������ ID.
 */
void AAOSPlayerState::ClearItemTimer(const int32 ItemID)
{
    FTimerHandle* TimerHandle = ItemTimerHandles.Find(ItemID);
    if (TimerHandle)
    {
        GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
        ItemTimerHandles.Remove(ItemID);
    }

    // ��ε�ĳ��Ʈ Ÿ�̸ӵ� ����
    FTimerHandle* BroadcastTimerHandle = BroadcastTimerHandles.Find(ItemID);
    if (BroadcastTimerHandle)
    {
        GetWorld()->GetTimerManager().ClearTimer(*BroadcastTimerHandle);
        BroadcastTimerHandles.Remove(ItemID);
    }
}


/**
 * ������ Ÿ�̸��� ���� �ð��� ��ȯ�մϴ�.
 *
 * �־��� ������ ID�� ���� Ÿ�̸��� ���� �ð��� ��ȯ�մϴ�.
 *
 * @param ItemID ���� �ð��� Ȯ���� Ÿ�̸��� ������ ID.
 * @return Ÿ�̸��� ���� �ð�(��) �Ǵ� Ÿ�̸Ӱ� ������ 0.
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