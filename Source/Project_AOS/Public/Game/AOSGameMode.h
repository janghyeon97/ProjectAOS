// 프로젝트 설정의 저작권 공지를 Description 페이지에 작성하십시오.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Structs/EnumTeamSide.h"
#include "Structs/EnumCharacterType.h"
#include "Structs/MinionData.h"
#include "Structs/GameData.h"
#include "Item/ItemData.h"
#include "AOSGameMode.generated.h"

class AAOSPlayerController;
class ACharacterBase;
class AAOSCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPlayersLoadedDelegate);

/**
 * AAOSGameMode 클래스는 플레이어 스포닝 및 게임 시작 로직을 포함한 주요 게임 흐름을 처리합니다.
 */
UCLASS()
class PROJECT_AOS_API AAOSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAOSGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	void PlayerLoaded(APlayerController* PlayerController);
	void GetLoadedItems(TArray<FItemInformation>& OutItems) const;
	void ActivateCurrencyIncrement();
	void ActivateSpawnMinion();
	void RequestRespawn(AAOSPlayerController* PlayerController);

	void OnCharacterDeath(AActor* InEliminator, TArray<ACharacterBase*> InNearbyPlayers, EObjectType InObjectType);
	void AddCurrencyToPlayer(ACharacterBase* Character, int32 Amount);
	void AddExpToPlayer(ACharacterBase* Character, int32 Amount);

private:
	void LoadGameData();
	void LoadItemData();
	void LoadPlayerData();
	void LoadMinionData();

	void StartGame();
	void SpawnMinion(EMinionType MinionType);
	void SpawnCharacter(AAOSPlayerController* PlayerController, int32 CharacterIndex, ETeamSideBase Team);
	void RespawnCharacter(AAOSPlayerController* PlayerController);
	void CheckAllPlayersLoaded();
	void FindPlayerStart();
	void SendLoadedItemsToClients();
	void IncrementPlayerCurrency();
	float CalculateRespawnTime(AAOSCharacterBase* Character) const;

	void SetTimer(TMap<int32, FTimerHandle>& Timers, TMap<int32, FTimerHandle>& BroadcastTimers, int32 TimerID, TFunction<void()> Callback, void (AAOSGameMode::* BroadcastFunc)(int32, float) const, float Duration, bool bLoop, float FirstDelay);
	void ClearTimer(TMap<int32, FTimerHandle>& Timers, TMap<int32, FTimerHandle>& BroadcastTimers, int32 TimerID);
	void BroadcastRemainingTime(int32 TimerID, float RemainingTime) const;
	void BroadcastRemainingRespawnTime(int32 PlayerIndex, float RemainingTime) const;
	float GetTimerRemaining(const TMap<int32, FTimerHandle>& Timers, int32 TimerID) const;

public:
	UPROPERTY()
	uint8 NumberOfPlayer = 1; // 게임 내 플레이어 수

	UPROPERTY()
	uint8 ConnectedPlayer; // 접속한 플레이어 수

	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	float MaxLoadWaitTime = 10.f;

	UPROPERTY()
	int32 InitialCharacterLevel = 1;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	TObjectPtr<class AAOSGameState> AOSGameState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMesh> WhiteMinion;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	TObjectPtr<class USkeletalMesh> BlackMinion;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	TMap<int32, AAOSPlayerController*> Players; // <PlayerIndex, AOSPlayerController>

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	TMap<AAOSPlayerController*, AAOSCharacterBase*> PlayerCharacterMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	TArray<TObjectPtr<AAOSPlayerController>> ExitingPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "GameData")
	TMap<int32, class AItem*> LoadedItems;

	UPROPERTY(BlueprintReadOnly, Category = "GameData")
	TMap<EMinionType, FMinionDataTableRow> LoadedMinions;

	UPROPERTY(BlueprintReadOnly, Category = "GameData")
	FGameDataTableRow LoadedGameData;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess))
	class UDataTable* ItemDataTable;

	UPROPERTY()
	TArray<AActor*> BlueTeamPlayerStart;

	UPROPERTY()
	TArray<AActor*> RedTeamPlayerStart;

	TArray<AActor*> PlayerStart;

	TMap<int32, FTimerHandle> GameTimers;					// <TimerID, TimerHandle>
	TMap<int32, FTimerHandle> RespawnTimers;				// <PlayerIndex, TimerHandle>
	TMap<int32, FTimerHandle> BroadcastGameTimerHandles;	// <PlayerIndex, TimerHandle>
	TMap<int32, FTimerHandle> BroadcastRespawnTimerHandles;	// <PlayerIndex, TimerHandle>

	FTimerHandle CurrencyIncrementTimerHandle;
	FTimerHandle ActivateSpawnMinionTimerHandle;
	FTimerHandle SpawnMinionTimerHandle;
	FTimerHandle LoadTimerHandle;

	int32 IncrementCurrencyAmount = 1;
	int32 NumberOfBlueTeam = 0;
	int32 NumberOfRedTeam = 0;

	const float MinionSpawnTime = 100000.f;
	const float MinionSpawnInterval = 15.f;


};