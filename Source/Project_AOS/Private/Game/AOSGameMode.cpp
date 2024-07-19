

#include "Game/AOSGameMode.h"
#include "Game/AOSGameInstance.h"
#include "Game/AOSGameState.h"
#include "Game/AOSPlayerState.h"
#include "Game/PlayerStateSave.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/MinionBase.h"
#include "Controllers/AOSPlayerController.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Item/Item.h"


AAOSGameMode::AAOSGameMode()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> ITEM_DATATABLE(TEXT("/Game/ProjectAOS/DataTables/DT_ItemList.DT_ItemList"));
	if (ITEM_DATATABLE.Succeeded()) ItemDataTable = ITEM_DATATABLE.Object;

	NumberOfPlayer = 0;
	ConnectedPlayer = 0;
	InitialCharacterLevel = 1;
}

void AAOSGameMode::BeginPlay()
{
	Super::BeginPlay();

	UAOSGameInstance* AOSGameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (::IsValid(AOSGameInstance))
	{
		NumberOfPlayer = AOSGameInstance->NumberOfPlayer >= 11 ? 1 : AOSGameInstance->NumberOfPlayer;
		UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::BeginPlay] Number of player %d"), NumberOfPlayer);
	}

	AOSGameState = GetGameState<AAOSGameState>();
	if (::IsValid(AOSGameState))
	{
		UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::BeginPlay] Successfully retrieved the GameState."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::BeginPlay] Failed to retrieve the GameState."));
	}

	LoadItemData();
	LoadMinionData();
	SendLoadedItemsToClients();

	GetWorldTimerManager().SetTimer(LoadTimerHandle, this, &AAOSGameMode::StartGame, MaxLoadWaitTime, false);
	GetWorldTimerManager().SetTimerForNextTick(this, &AAOSGameMode::CheckAllPlayersLoaded);
}

void AAOSGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (auto& ItemPair : LoadedItems)
	{
		if (ItemPair.Value)
		{
			ItemPair.Value->ConditionalBeginDestroy();
		}
	}
	LoadedItems.Empty();

	GetWorldTimerManager().ClearTimer(CurrencyIncrementTimerHandle);
}

void AAOSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AAOSPlayerController* NewPlayerController = Cast<AAOSPlayerController>(NewPlayer);
	if (::IsValid(NewPlayerController))
	{
		AAOSPlayerState* AOSPlayerState = NewPlayerController->GetPlayerState<AAOSPlayerState>();
		if (::IsValid(AOSPlayerState))
		{
			FString SaveSlotName = AOSPlayerState->GetPlayerUniqueID();
			UPlayerStateSave* PlayerStateSave = Cast<UPlayerStateSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
			if (::IsValid(PlayerStateSave) == false)
			{
				PlayerStateSave = GetMutableDefault<UPlayerStateSave>();
			}

			AOSPlayerState->TeamSide = PlayerStateSave->TeamSide;
			AOSPlayerState->SetPlayerIndex(PlayerStateSave->PlayerIndex);
			AOSPlayerState->SetSelectedChampionIndex(PlayerStateSave->SelectedChampionIndex);
			AOSPlayerState->SetPlayerUniqueID(PlayerStateSave->PlayerUniqueID);

			Players.Add(PlayerStateSave->PlayerIndex, NewPlayerController);

			UE_LOG(LogTemp, Warning, TEXT("[AAOSGameMode::PostLogin] %s Player LoggedIn(%d, %d, %s)  %d/%d"),
				*AOSPlayerState->GetPlayerName(), AOSPlayerState->GetPlayerIndex(),
				AOSPlayerState->GetSelectedChampionIndex(), *AOSPlayerState->GetPlayerUniqueID(),
				ConnectedPlayer, NumberOfPlayer);
		}
	}
}

void AAOSGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AAOSPlayerController* ExitingPlayerController = Cast<AAOSPlayerController>(Exiting);
	if (::IsValid(ExitingPlayerController))
	{
		if (AAOSPlayerState* AOSPlayerState = ExitingPlayerController->GetPlayerState<AAOSPlayerState>())
		{
			int32 PlayerIndex = AOSPlayerState->GetPlayerIndex();
			if (nullptr != Players.Find(PlayerIndex))
			{
				Players.Remove(PlayerIndex);
				ExitingPlayers.Add(ExitingPlayerController);
			}
		}
	}
}

void AAOSGameMode::PlayerLoaded(APlayerController* PlayerController)
{
	ConnectedPlayer++;
}

void AAOSGameMode::LoadGameData()
{
	UAOSGameInstance* AOSGameInstance = Cast<UAOSGameInstance>(GetGameInstance());
	if (!AOSGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::LoadGameData] Invalid GameInstance."));
		return;
	}

	const UDataTable* DataTable = AOSGameInstance->GetGameDataTable();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::LoadGameData] Invalid DataTable."));
		return;
	}

	LoadedGameData = *DataTable->FindRow<FGameDataTableRow>(FName(*FString::FromInt(1)), TEXT(""));
}

void AAOSGameMode::LoadItemData()
{
	if (ItemDataTable)
	{
		LoadedItems.Empty();

		static const FString ContextString(TEXT("GENERAL"));
		TArray<FItemTableRow*> Items;
		ItemDataTable->GetAllRows<FItemTableRow>(ContextString, Items);

		if (Items.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AAOSGameMode::LoadItemData] No items found in ItemDataTable."));
		}

		for (auto& ItemRow : Items)
		{
			if (ItemRow && ItemRow->ItemClass)
			{
				AItem* ItemInstance = NewObject<AItem>(this, ItemRow->ItemClass);
				if (ItemInstance)
				{
					ItemInstance->ItemID = ItemRow->ItemID;
					ItemInstance->Name = ItemRow->Name;
					ItemInstance->Price = ItemRow->Price;
					ItemInstance->Icon = ItemRow->Icon;
					ItemInstance->Description = ItemRow->Description;
					ItemInstance->MaxStack = ItemRow->ItemStackLimit;
					ItemInstance->MaxPossessQuantity = ItemRow->MaxPossessQuantity;
					ItemInstance->Classification = ItemRow->Classification;
					ItemInstance->Abilities = ItemRow->Abilities;
					ItemInstance->RequiredItems = ItemRow->RequiredItems;

					LoadedItems.Add(ItemInstance->ItemID, ItemInstance);

					if (!LoadedItems.Contains(ItemInstance->ItemID))
					{
						LoadedItems.Add(ItemInstance->ItemID, ItemInstance);
						UE_LOG(LogTemp, Log, TEXT("Loaded Item: %d, %s, %d, %s"),
							ItemInstance->ItemID,
							*ItemInstance->Name,
							ItemInstance->Price,
							*ItemInstance->Description);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[AAOSGameMode::LoadItemData] Duplicate ItemID found: %d"), ItemInstance->ItemID);
					}
				}
			}
		}
	}
}

void AAOSGameMode::GetLoadedItems(TArray<FItemInformation>& OutItems) const
{
	for (const auto& ItemPair : LoadedItems)
	{
		if (ItemPair.Value)
		{
			FItemInformation Item;
			Item.ItemID = ItemPair.Value->ItemID;
			Item.Name = ItemPair.Value->Name;
			Item.Price = ItemPair.Value->Price;
			Item.Description = ItemPair.Value->Description;
			Item.Icon = ItemPair.Value->Icon;
			Item.ItemStackLimit = ItemPair.Value->MaxStack;
			Item.CurrentStack = ItemPair.Value->CurrentStack;
			Item.Classification = ItemPair.Value->Classification;
			Item.Abilities = ItemPair.Value->Abilities;
			Item.RequiredItems = ItemPair.Value->RequiredItems;

			OutItems.Add(Item);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::GetLoadedItems] Item index %d is not valid."), ItemPair.Key);
		}
	}
}

void AAOSGameMode::SendLoadedItemsToClients()
{
	if (AOSGameState)
	{
		TArray<FItemInformation> Items;
		GetLoadedItems(Items);
		AOSGameState->SetLoadedItems(Items);
	}
}

void AAOSGameMode::LoadPlayerData()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		AAOSPlayerState* PlayerState = Cast<AAOSPlayerState>(PlayerController->PlayerState);

		if (PlayerState)
		{
			FString SaveSlotName = FString::FromInt(PlayerState->GetUniqueID());
			UPlayerStateSave* PlayerStateSave = Cast<UPlayerStateSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
			if (false == ::IsValid(PlayerStateSave))
			{
				PlayerStateSave = GetMutableDefault<UPlayerStateSave>();
			}

			PlayerState->SetTeamSide(PlayerStateSave->TeamSide);
			PlayerState->SetPlayerIndex(PlayerStateSave->PlayerIndex);
			PlayerState->SetSelectedChampionIndex(PlayerStateSave->SelectedChampionIndex);
		}
	}
}

void AAOSGameMode::LoadMinionData()
{
	UAOSGameInstance* AOSGameInstance = Cast<UAOSGameInstance>(GetGameInstance());
	if (!AOSGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::LoadMinionData] Invalid GameInstance."));
		return;
	}

	const UDataTable* DataTable = AOSGameInstance->GetMinionDataTable();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::LoadMinionData] Invalid DataTable."));
		return;
	}

	// 기존 데이터 클리어
	LoadedMinions.Empty();
		
	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FMinionDataTableRow* MinionData = DataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
		if (!MinionData)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AAOSGameMode::LoadMinionData] Failed to find row: %s"), *RowName.ToString());
			continue;
		}

		EMinionType MinionType = MinionData->MinionType;
		LoadedMinions.Add(MinionType, *MinionData);
	}
}

void AAOSGameMode::CheckAllPlayersLoaded()
{
	bool bAllPlayersLoaded = (NumberOfPlayer == ConnectedPlayer);

	if (bAllPlayersLoaded)
	{
		UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::CheckAllPlayersLoaded] All players have loaded. Starting the game."));
		GetWorldTimerManager().ClearTimer(LoadTimerHandle);
		StartGame();
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AAOSGameMode::CheckAllPlayersLoaded);
	}
}

void AAOSGameMode::StartGame()
{
	if (::IsValid(AOSGameState) == false)
	{
		AOSGameState = Cast<AAOSGameState>(GameState);
		GetWorldTimerManager().SetTimerForNextTick(this, &AAOSGameMode::StartGame);
		return;
	}

	if (Players.Num() > 0)
	{
		for (const TPair<int32, AAOSPlayerController*>& Pair : Players)
		{
			AAOSPlayerController* PC = Pair.Value;

			if (!PC)
			{
				continue;
			}

			AAOSPlayerState* AOSPlayerState = PC->GetPlayerState<AAOSPlayerState>();

			if (!AOSPlayerState)
			{
				continue;
			}

			UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::CreatePlayerCharacter()] Create %s Player's Character - %d"),
				*AOSPlayerState->GetPlayerName(),
				AOSPlayerState->GetSelectedChampionIndex());

			int32 NewIndex = AOSPlayerState->GetSelectedChampionIndex();

			SpawnCharacter(PC, NewIndex > 0 ? NewIndex : 1, AOSPlayerState->TeamSide);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::StartGame] Start Game."));

	int32 GameTimerID = FMath::Rand();
	auto TimerCallback = [this]() { ActivateSpawnMinion(); };

	AOSGameState->StartGame();
	SetTimer(GameTimers, BroadcastGameTimerHandles, GameTimerID, TimerCallback, &AAOSGameMode::BroadcastRemainingTime, MinionSpawnInterval, true, MinionSpawnTime);
	ActivateCurrencyIncrement();
}

void AAOSGameMode::SpawnCharacter(AAOSPlayerController* PlayerController, int32 ChampionIndex, ETeamSideBase Team)
{
	FindPlayerStart();

	if (PlayerStart.Num() > 0 && PlayerStart.IsValidIndex(0))
	{
		UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(GetGameInstance());
		if (!::IsValid(GameInstance))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid GameInstance."));
			return;
		}

		FChampionsListRow* CharacterData = GameInstance->GetCampionsListTableRow(ChampionIndex);
		if (!CharacterData || !CharacterData->CharacterClass)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid CharacterIndex or CharacterClass."));
			return;
		}

		AAOSCharacterBase* Character = GetWorld()->SpawnActor<AAOSCharacterBase>(CharacterData->CharacterClass, PlayerStart[0]->GetActorTransform());
		if (!::IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn character."));
			return;
		}

		if (::IsValid(AOSGameState))
		{
			AOSGameState->AddPlayerCharacter(Character, Team);
		}

		PlayerController->Possess(Character);
		PlayerController->RemoveLoadingScreen();
		PlayerCharacterMap.FindOrAdd(PlayerController, Character);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid PlayerStart."));
	}
}

void AAOSGameMode::SpawnMinion(EMinionType MinionType)
{
	FMinionDataTableRow* MinionDataPtr = LoadedMinions.Find(MinionType);
	if (!MinionDataPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] Invalid MinionType: %d"), (int32)MinionType);
		return;
	}

	// PlayerStart 배열의 유효성 확인
	if (PlayerStart.Num() == 0 || !PlayerStart.IsValidIndex(0))
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] Invalid PlayerStart."));
		return;
	}

	// Minion 스폰 로직
	FTransform SpawnTransform = PlayerStart[0]->GetActorTransform();
	AMinionBase* NewMinion = Cast<AMinionBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, MinionDataPtr->MinionClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
	if (NewMinion)
	{
		// SkeletalMeshComponent 유효성 확인
		if (MinionDataPtr->SkeletalMesh_Down)
		{
			NewMinion->ReplicatedSkeletalMesh = MinionDataPtr->SkeletalMesh_Down;
			NewMinion->TeamSide = ETeamSideBase::Blue;

			NewMinion->ExperienceShareRadius = LoadedGameData.ExperienceShareRadius;
			NewMinion->ShareFactor.Add(1, 1.0f);
			NewMinion->ShareFactor.Add(2, LoadedGameData.ExpShareFactorTwoPlayers);
			NewMinion->ShareFactor.Add(3, LoadedGameData.ExpShareFactorThreePlayers);
			NewMinion->ShareFactor.Add(4, LoadedGameData.ExpShareFactorFourPlayers);
			NewMinion->ShareFactor.Add(5, LoadedGameData.ExpShareFactorFivePlayers);

			NewMinion->SetExpBounty(MinionDataPtr->ExpBounty);
			NewMinion->SetGoldBounty(MinionDataPtr->GoldBounty);
			NewMinion->SetAnimMontages(MinionDataPtr->Montages);
		}

		UGameplayStatics::FinishSpawningActor(NewMinion, SpawnTransform);
		UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::SpawnMinion] Spawned minion of type: %d"), (int32)MinionType);
	}
}



void AAOSGameMode::IncrementPlayerCurrency()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (PlayerController)
		{
			AAOSPlayerState* PlayerState = Cast<AAOSPlayerState>(PlayerController->PlayerState);
			if (PlayerState)
			{
				PlayerState->SetCurrency(PlayerState->GetCurrency() + IncrementCurrencyAmount);
			}
		}
	}
}

void AAOSGameMode::ActivateCurrencyIncrement()
{
	GetWorldTimerManager().SetTimer(CurrencyIncrementTimerHandle, this, &AAOSGameMode::IncrementPlayerCurrency, 1.0f, true);
}

void AAOSGameMode::ActivateSpawnMinion()
{
	for (int32 i = 0; i < 3; i++)
	{
		SpawnMinion(EMinionType::Melee);
	}
}

void AAOSGameMode::RequestRespawn(AAOSPlayerController* PlayerController)
{
	if (!::IsValid(PlayerController))
	{
		return;
	}

	AAOSCharacterBase* Character = Cast<AAOSCharacterBase>(PlayerController->GetPawn());
	if (!::IsValid(Character))
	{
		return;
	}

	AAOSPlayerState* AOSPlayerState = PlayerController->GetPlayerState<AAOSPlayerState>();
	if (!AOSPlayerState)
	{
		return;
	}

	float RespawnTime = CalculateRespawnTime(Character);

	int32 TimerID = AOSPlayerState->GetPlayerIndex();
	auto TimerCallback = [this, WeakPlayerController = TWeakObjectPtr<AAOSPlayerController>(PlayerController)]()
		{ 
			if (AAOSPlayerController* ValidPlayerController = WeakPlayerController.Get())
			{
				RespawnCharacter(ValidPlayerController);
			}
		};

	AOSGameState->StartGame();
	SetTimer(RespawnTimers, BroadcastRespawnTimerHandles, TimerID, TimerCallback, &AAOSGameMode::BroadcastRemainingRespawnTime, 1.0f, false, RespawnTime);
}

void AAOSGameMode::RespawnCharacter(AAOSPlayerController* PlayerController)
{
	if (!::IsValid(PlayerController))
	{
		return;
	}

	AAOSPlayerState* AOSPlayerState = PlayerController->GetPlayerState<AAOSPlayerState>();
	if (!AOSPlayerState)
	{
		return;
	}

	AAOSCharacterBase** CharacterPtr = PlayerCharacterMap.Find(PlayerController);
	if (CharacterPtr && ::IsValid(*CharacterPtr))
	{
		(*CharacterPtr)->Respawn();
		ClearTimer(RespawnTimers, BroadcastRespawnTimerHandles, AOSPlayerState->GetPlayerIndex());
	}
	else
	{
		// 새로운 캐릭터 스폰 로직 필요
	}
}

float AAOSGameMode::CalculateRespawnTime(AAOSCharacterBase* Character) const
{
	if (!AOSGameState)
	{
		return 5.0f; // 기본 리스폰 시간
	}

	if (UStatComponent* StatComponent = Character->GetStatComponent())
	{
		float ElapsedTime = AOSGameState->GetElapsedTime();
		int32 CharacterLevel = StatComponent->GetCurrentLevel();

		// 리스폰 시간을 계산하는 로직
		float BaseRespawnTime = 5.0f;
		float TimeFactor = ElapsedTime / 60.0f; // 분 단위로 증가
		float LevelFactor = CharacterLevel * 1.f; // 레벨 당 1초 증가

		return BaseRespawnTime + TimeFactor + LevelFactor;
	}

	return 5.f;
}

void AAOSGameMode::SetTimer(TMap<int32, FTimerHandle>& Timers, TMap<int32, FTimerHandle>& BroadcastTimers, int32 TimerID, TFunction<void()> Callback, void (AAOSGameMode::* BroadcastFunc)(int32, float) const, float Duration, bool bLoop, float FirstDelay)
{
	FTimerHandle& TimerHandle = Timers.FindOrAdd(TimerID);
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(Callback);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, bLoop, FirstDelay);

	// 주기적으로 남은 시간을 브로드캐스트하는 타이머 설정
	FTimerHandle& BroadcastTimerHandle = BroadcastTimers.FindOrAdd(TimerID);
	GetWorld()->GetTimerManager().SetTimer(BroadcastTimerHandle, [this, &Timers, TimerID, BroadcastFunc]()
		{
			float RemainingTime = GetTimerRemaining(Timers, TimerID);
			(this->*BroadcastFunc)(TimerID, RemainingTime);
		}, 1.0f, true, 0.0f);
}


void AAOSGameMode::ClearTimer(TMap<int32, FTimerHandle>& Timers, TMap<int32, FTimerHandle>& BroadcastTimers, int32 TimerID)
{
	if (FTimerHandle* TimerHandle = Timers.Find(TimerID))
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
		Timers.Remove(TimerID);
	}

	if (FTimerHandle* BroadcastTimerHandle = BroadcastTimers.Find(TimerID))
	{
		GetWorld()->GetTimerManager().ClearTimer(*BroadcastTimerHandle);
		BroadcastTimers.Remove(TimerID);
	}
}

float AAOSGameMode::GetTimerRemaining(const TMap<int32, FTimerHandle>& Timers, int32 TimerID) const
{
	if (const FTimerHandle* TimerHandle = Timers.Find(TimerID))
	{
		return GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandle);
	}
	return 0.f;
}

void AAOSGameMode::BroadcastRemainingTime(int32 TimerID, float RemainingTime) const
{

}

void AAOSGameMode::BroadcastRemainingRespawnTime(int32 PlayerIndex, float RemainingTime) const
{
	if (!::IsValid(AOSGameState))
	{
		return;
	}

	AOSGameState->MulticastBroadcastRespawnTime(PlayerIndex, RemainingTime);
}

void AAOSGameMode::FindPlayerStart()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStart);
}


void AAOSGameMode::OnCharacterDeath(AActor* InEliminator, TArray<ACharacterBase*> InNearbyPlayers, EObjectType InObjectType)
{
	ACharacterBase* Eliminator = Cast<ACharacterBase>(InEliminator);
	if (!Eliminator)
	{
		return;
	}

	if (EnumHasAnyFlags(Eliminator->ObjectType, EObjectType::Player))
	{
		switch (InObjectType)
		{
		case EObjectType::Player:
			
			break;
		case EObjectType::Minion:
			break;
		default:
			break;
		}
	}
	else
	{
		switch (InObjectType)
		{
		case EObjectType::Player:
			break;
		case EObjectType::Minion:
			break;
		default:
			break;
		}
	}
}

void AAOSGameMode::AddCurrencyToPlayer(ACharacterBase* Character, int32 Amount)
{
	if (::IsValid(Character) == false)
	{
		return;
	}

	UStatComponent* StatComopnent = Character->GetStatComponent();
	if (!StatComopnent)
	{
		return;
	}

	AAOSPlayerState* PlayerState = Character->GetPlayerState<AAOSPlayerState>();
	if (!PlayerState)
	{
		return;
	}

	int32 CurrentGold = PlayerState->GetCurrency();
	PlayerState->SetCurrency(CurrentGold + Amount);
}

void AAOSGameMode::AddExpToPlayer(ACharacterBase* Character, int32 Amount)
{
	if (::IsValid(Character) == false)
	{
		return;
	}

	UStatComponent* StatComopnent = Character->GetStatComponent();
	if (!StatComopnent)
	{
		return;
	}

	int32 CurrentExp = StatComopnent->GetCurrentEXP();
	StatComopnent->SetCurrentEXP(CurrentExp + Amount);
}
