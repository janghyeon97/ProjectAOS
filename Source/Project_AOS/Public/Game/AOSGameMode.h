
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Structs/CharacterResources.h"
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

	int32 GetInitialCharacterLevel() const { return InitialCharacterLevel; };

private:
	void LoadGameData();
	void LoadItemData();
	void LoadPlayerData();
	void LoadMinionData();
	FCharacterGamePlayDataRow CacheCharacterResources(UDataTable* ResourcesTable);

	void StartGame();
	void SpawnMinionsForLane(const FName& Lane);
	void SpawnMinion(EMinionType MinionType, const FString& Lane, ETeamSideBase Team);
	void SpawnCharacter(AAOSPlayerController* PlayerController, const FName& ChampionRowName, ETeamSideBase Team, const int32 PlayerIndex);
	void RespawnCharacter(AAOSPlayerController* PlayerController);
	void CheckAllPlayersLoaded();
	void FindPlayerStart();
	void FindSplinePath();
	void SendLoadedItemsToClients();
	void IncrementPlayerCurrency();
	float CalculateRespawnTime(AAOSCharacterBase* Character) const;

	void SetTimer(TMap<int32, FTimerHandle>& Timers, TMap<int32, FTimerHandle>& BroadcastTimers, int32 TimerID, TFunction<void()> Callback, void (AAOSGameMode::* BroadcastFunc)(int32, float) const, float Duration, bool bLoop, float FirstDelay);
	void ClearTimer(TMap<int32, FTimerHandle>& Timers, TMap<int32, FTimerHandle>& BroadcastTimers, int32 TimerID);
	void BroadcastRemainingTime(int32 TimerID, float RemainingTime) const;
	void BroadcastRemainingRespawnTime(int32 PlayerIndex, float RemainingTime) const;
	float GetTimerRemaining(const TMap<int32, FTimerHandle>& Timers, int32 TimerID) const;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AAOSGameState> AOSGameState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess = "true"))
	TMap<int32, AAOSPlayerController*> Players; // <PlayerIndex, AOSPlayerController>

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess = "true"))
	TMap<AAOSPlayerController*, AAOSCharacterBase*> PlayerCharacterMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AAOSPlayerController>> ExitingPlayers;

public:
	UPROPERTY(BlueprintReadOnly, Category = "GameData", Meta = (AllowPrivateAccess = "true"))
	TMap<int32, class AItem*> LoadedItems;

	UPROPERTY(BlueprintReadOnly, Category = "GameData", Meta = (AllowPrivateAccess = "true"))
	TMap<EMinionType, FMinionDataTableRow> LoadedMinionData;

	UPROPERTY(BlueprintReadOnly, Category = "GameData", Meta = (AllowPrivateAccess = "true"))
	FGameDataTableRow LoadedGameData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AOSGameMode", Meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOSGameMode", Meta = (AllowPrivateAccess = "true"))
	TMap<FString, AActor*> SplinePaths;

private:
	float MaxLoadWaitTime = 10.f;
	uint8 NumberOfPlayer = 1; // 게임 내 플레이어 수
	uint8 ConnectedPlayer; // 접속한 플레이어 수
	int32 InitialCharacterLevel = 1;

	FName DefaultCharacter = "Sparrow";

	TMap<FName, APlayerStart*> PlayerStarts;
	TMap<FName, APlayerStart*> MinionStarts;

	TMap<int32, FTimerHandle> GameTimers;					// <TimerID, TimerHandle>
	TMap<int32, FTimerHandle> RespawnTimers;				// <PlayerIndex, TimerHandle>
	TMap<int32, FTimerHandle> BroadcastGameTimerHandles;	// <PlayerIndex, TimerHandle>
	TMap<int32, FTimerHandle> BroadcastRespawnTimerHandles;	// <PlayerIndex, TimerHandle>

	FTimerHandle CurrencyIncrementTimerHandle;
	FTimerHandle MinionSpawnTimerHandle;
	FTimerHandle MinionSpawnTimerHandle2;
	FTimerHandle LoadTimerHandle;

	int32 SpawnCount = 0;
	int32 NumberOfBlueTeam = 0;
	int32 NumberOfRedTeam = 0;

	// 클래스 수준의 캐시된 리소스
	static TMap<EMinionType, FCharacterGamePlayDataRow> CachedCharacterResources;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CrowdControl", meta = (AllowPrivateAccess = "true"))
	class UCrowdControlManager* CrowdControlManager;
};
