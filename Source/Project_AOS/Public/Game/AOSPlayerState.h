#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Structs/EnumTeamSide.h"
#include "AOSPlayerState.generated.h"

class AItem;

DECLARE_MULTICAST_DELEGATE(FOnPlayerInfoUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPurchasedDelegate, int32, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedDelegate, const TArray<FItemInformation>&, Items);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyUpdatedDelegate, const int32, NewCurrency);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemTimerUpdatedDelegate, int32, ItemID, float, RemainingTime);

UCLASS()
class PROJECT_AOS_API AAOSPlayerState : public APlayerState
{
	GENERATED_BODY()

	friend class ALobbyGameMode;

public:
	AAOSPlayerState();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetOwner(AActor* NewOwner);

public:
	// Accessors
	int32 GetAbilityPoints() const { return AbilityPoints; }
	void SetAbilityPoints(int32 InAbilityPoints) { AbilityPoints = InAbilityPoints; }

	ETeamSideBase GetTeamSide() const { return TeamSide; }
	void SetTeamSide(ETeamSideBase NewTeamSide) { TeamSide = NewTeamSide; }

	FName GetPlayerUniqueID() const;
	void SetPlayerUniqueID(const FName& NewPlayerUniqueID) { PlayerUniqueID = NewPlayerUniqueID; }

	int32 GetPlayerIndex() const { return PlayerIndex; }
	void SetPlayerIndex(int32 NewPlayerIndex) { PlayerIndex = NewPlayerIndex; }

	int32 GetSelectedChampionIndex() const { return SelectedChampionIndex; }
	void SetSelectedChampionIndex(int32 NewSelectedChampionIndex) { SelectedChampionIndex = NewSelectedChampionIndex; }

	FName GetSelectedChampionName() const { return SelectedChampionName; }
	void SetSelectedChampionName(const FName& NewSelectedChampionName) { SelectedChampionName = NewSelectedChampionName; }

	int32 GetCurrency() const { return Currency; }
	void SetCurrency(int32 NewCurrency) { Currency = NewCurrency; }

	// Item Timer Management
	bool IsItemTimerActive(int32 ItemID) const;
	void SetItemTimer(int32 ItemID, TFunction<void()> Callback, float Duration, bool bLoop, float FirstDelay = -1.f);
	void ClearItemTimer(int32 ItemID);
	float GetItemTimerRemaining(int32 ItemID) const;

	// Inventory Management
	void AddItemToInventory(AItem* Item);
	void RemoveItemFromInventory(int32 ItemID);
	void ApplyDiscountAndRemoveSubItems(AItem* Item, int32& OutFinalPrice);
	void ApplyDiscount(AItem* Item, int32& OutFinalPrice);
	void BindItem(AItem* Item);
	void SwapItemsInInventory(int32 Index1, int32 Index2);

	// Item Usage
	UFUNCTION(BlueprintCallable, Category = "Player")
	void UseItem(int32 Index);

	UFUNCTION()
	void SendLoadedItemsToClients();

	// Server Functions
	UFUNCTION(Server, Reliable)
	void UpdateSelectedChampion_Server(int32 Index);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Player")
	void ServerPurchaseItem(int32 ItemID);

	// Events
	UFUNCTION()
	void OnPlayerLevelChanged(int32 OldLevel, int32 NewLevel);

	// Delegate
	FOnPlayerInfoUpdatedDelegate OnPlayerInfoUpdated;
	FOnItemPurchasedDelegate OnItemPurchased;
	FOnInventoryUpdatedDelegate OnInventoryUpdated;
	FOnCurrencyUpdatedDelegate OnCurrencyUpdated;
	FOnItemTimerUpdatedDelegate OnItemTimerUpdated;

private:
	// Helper Functions
	int32 GetTotalItemCount(int32 ItemID) const;
	bool CanAddItemWithSubItems(AItem* Item);
	bool IsInventoryFull() const;
	void ConvertInventoryItems(TArray<FItemInformation>& OutItems) const;

	UFUNCTION(Client, Unreliable)
	void BroadcastRemainingTime(int32 ItemID, float RemainingTime);

	// Inventory Replication
	UFUNCTION(Client, Reliable)
	void ClientInventoryChanged(const TArray<FItemInformation>& Items);

	UFUNCTION(Client, Reliable)
	void ClientOnItemPurchased(int32 ItemID);

	UFUNCTION()
	void OnRep_CurrencyUpdated();

public:
	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ETeamSideBase TeamSide;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName PlayerUniqueID;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 PlayerIndex;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 SelectedChampionIndex;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName SelectedChampionName;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player", meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;
	
	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 AbilityPoints;

	UPROPERTY(ReplicatedUsing = OnRep_CurrencyUpdated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 Currency;

	// Timer Handles
	TMap<int32, FTimerHandle> ItemTimerHandles;			// <ItemID, TimerHandle>
	TMap<int32, FTimerHandle> BroadcastTimerHandles;	// <ItemID, TimerHandle>
};