// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Structs/EnumTeamSide.h"
#include "Item/ItemData.h"
#include "AOSGameState.generated.h"


class AAOSCharacterBase;

USTRUCT(BlueprintType)
struct FPlayerInformation
{
	GENERATED_BODY()

public:
	FPlayerInformation()
	{

	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AAOSCharacterBase* Character;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChampionIndex;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadedItemsUpdatedDelegate, const TArray<FItemInformation>&, Items);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRespawnTimerUpdatedDelegate, int32, PlayerIndex, float, RemainingTime);
DECLARE_MULTICAST_DELEGATE(FOnPlayerCharactersReplicatedDelegate);


/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AAOSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AAOSGameState();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	TArray<TObjectPtr<AAOSCharacterBase>> GetBlueTeamPlayers() { return BlueTeamPlayers; };
	TArray<TObjectPtr<AAOSCharacterBase>> GetRedTeamPlayers() { return RedTeamPlayers; };

	void SetLoadedItems(const TArray<FItemInformation>& Items) { LoadedItems = Items; };
	const TArray<FItemInformation>& GetLoadedItems() { return LoadedItems; };

	float GetElapsedTime() const { return ElapsedTime; };
	FItemInformation* GetItemInfoByID(int32 ItemID);

	void StartGame();
	void AddPlayerCharacter(AAOSCharacterBase* Character, ETeamSideBase TeamSide);
	void RemovePlayerCharacter(AAOSCharacterBase* Character);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastRespawnTime(int32 PlayerIndex, float RemainingTime);

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSendLoadedItems(const TArray<FItemInformation>& Items);

	UFUNCTION()
	void OnRep_PlayerCharactersReplicated();

	UFUNCTION()
	void OnRep_LoadedItemsReplicated();

	FOnPlayerCharactersReplicatedDelegate OnPlayerCharactersReplicated;
	FOnRespawnTimerUpdatedDelegate OnRespawnTimerUpdated;
	FOnLoadedItemsUpdatedDelegate OnLoadedItemsUpdated;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AOSGameState", Meta = (AllowPrivateAccess))
	TObjectPtr<class AAOSGameMode> AOSGameMode;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerCharactersReplicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "AOSGameState", Meta = (AllowPrivateAccess))
	TArray<TObjectPtr<AAOSCharacterBase>> BlueTeamPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerCharactersReplicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "AOSGameState", Meta = (AllowPrivateAccess))
	TArray<TObjectPtr<AAOSCharacterBase>> RedTeamPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_LoadedItemsReplicated, VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	TArray<FItemInformation> LoadedItems;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AOSGameState", Meta = (AllowPrivateAccess))
	float StartTime = 0.f;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "AOSGameState", Meta = (AllowPrivateAccess))
	float ElapsedTime = 0.f;

	TMap<AAOSCharacterBase*, int32> RespawnTime;

	bool bIsGameStarted = false;
};