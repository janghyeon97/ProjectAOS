#include "Game/AOSGameMode.h"
#include "Game/AOSGameInstance.h"
#include "Game/AOSGameState.h"
#include "Game/AOSPlayerState.h"
#include "Game/PlayerStateSave.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/MinionBase.h"
#include "Controllers/AOSPlayerController.h"
#include "Controllers/MinionAIController.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "Kismet/GameplayStatics.h"
#include "Item/Item.h"

// Ŭ���� ������ ĳ�õ� ���ҽ� �ʱ�ȭ
TMap<EMinionType, FCharacterGamePlayDataRow> AAOSGameMode::CachedCharacterResources;

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

    CrowdControlManager = UCrowdControlManager::Get();
    if (CrowdControlManager)
    {
        UE_LOG(LogTemp, Log, TEXT("CrowdControlManager initialized successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize CrowdControlManager."));
    }

    LoadGameData();
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

    UCrowdControlManager::Release();

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
            FName SaveSlotName = AOSPlayerState->GetPlayerUniqueID();
            UPlayerStateSave* PlayerStateSave = Cast<UPlayerStateSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName.ToString(), 0));
            if (!::IsValid(PlayerStateSave))
            {
                PlayerStateSave = GetMutableDefault<UPlayerStateSave>();
            }

            AOSPlayerState->TeamSide = PlayerStateSave->TeamSide;
            AOSPlayerState->SetPlayerIndex(PlayerStateSave->PlayerIndex);
            AOSPlayerState->SetSelectedChampionIndex(PlayerStateSave->SelectedChampionIndex);
            AOSPlayerState->SetSelectedChampionName(PlayerStateSave->SelectedChampionName);
            AOSPlayerState->SetPlayerUniqueID(PlayerStateSave->PlayerUniqueID);

            Players.Add(PlayerStateSave->PlayerIndex, NewPlayerController);

            UE_LOG(LogTemp, Warning, TEXT("[AAOSGameMode::PostLogin] %s Player LoggedIn(%d, %d, %s)  %d/%d"),
                *AOSPlayerState->GetPlayerName(), AOSPlayerState->GetPlayerIndex(),
                AOSPlayerState->GetSelectedChampionIndex(), *AOSPlayerState->GetPlayerUniqueID().ToString(),
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
            if (Players.Find(PlayerIndex))
            {
                Players.Remove(PlayerIndex);
                ExitingPlayers.Add(ExitingPlayerController);
            }
        }
    }
}

void AAOSGameMode::LoadGameData()
{
    UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!::IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::LoadGameData] Invalid GameInstance."));
        return;
    }

    const UDataTable* DataTable = GameInstance->GetGameDataTable();
    if (!DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::LoadGameData] Invalid DataTable."));
        return;
    }

    FGameDataTableRow* DataRow = DataTable->FindRow<FGameDataTableRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::LoadGameData] Failed to find row in DataTable."));
        return;
    }

    LoadedGameData = *DataRow;

    // LoadedGameData�� �α׷� ���
    UE_LOG(LogTemp, Log, TEXT("Loaded Game Data:"));
    UE_LOG(LogTemp, Log, TEXT("  IncrementCurrencyAmount: %d"), LoadedGameData.IncrementCurrencyAmount);
    UE_LOG(LogTemp, Log, TEXT("  MaxAssistTime: %f"), LoadedGameData.MaxAssistTime);
    UE_LOG(LogTemp, Log, TEXT("  ExperienceShareRadius: %f"), LoadedGameData.ExperienceShareRadius);
    UE_LOG(LogTemp, Log, TEXT("  ExpShareFactorTwoPlayers: %f"), LoadedGameData.ExpShareFactorTwoPlayers);
    UE_LOG(LogTemp, Log, TEXT("  ExpShareFactorThreePlayers: %f"), LoadedGameData.ExpShareFactorThreePlayers);
    UE_LOG(LogTemp, Log, TEXT("  ExpShareFactorFourPlayers: %f"), LoadedGameData.ExpShareFactorFourPlayers);
    UE_LOG(LogTemp, Log, TEXT("  ExpShareFactorFivePlayers: %f"), LoadedGameData.ExpShareFactorFivePlayers);
    UE_LOG(LogTemp, Log, TEXT("  MinionsPerWave: %d"), LoadedGameData.MinionsPerWave);
    UE_LOG(LogTemp, Log, TEXT("  SuperMinionSpawnInterval: %d"), LoadedGameData.SuperMinionSpawnInterval);
    UE_LOG(LogTemp, Log, TEXT("  SpawnInterval: %f"), LoadedGameData.SpawnInterval);
    UE_LOG(LogTemp, Log, TEXT("  MinionSpawnTime: %f"), LoadedGameData.MinionSpawnTime);
    UE_LOG(LogTemp, Log, TEXT("  MinionSpawnInterval: %f"), LoadedGameData.MinionSpawnInterval);
}


void AAOSGameMode::PlayerLoaded(APlayerController* PlayerController)
{
    ConnectedPlayer++;
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
    if (!::IsValid(AOSGameState))
    {
        AOSGameState = Cast<AAOSGameState>(GameState);
        GetWorldTimerManager().SetTimerForNextTick(this, &AAOSGameMode::StartGame);
        return;
    }

    FindPlayerStart();
    FindSplinePath();

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

            FName ChampionRowName = AOSPlayerState->GetSelectedChampionName();
            ChampionRowName = !ChampionRowName.IsNone() ? ChampionRowName : DefaultCharacter;

            ETeamSideBase Team = AOSPlayerState->TeamSide == ETeamSideBase::Neutral || AOSPlayerState->TeamSide == ETeamSideBase::Type
                ? ETeamSideBase::Blue : AOSPlayerState->TeamSide;

            UE_LOG(LogTemp, Log, TEXT("Assigned Team: %s"), Team == ETeamSideBase::Blue ? TEXT("Blue") : TEXT("Red"));

            SpawnCharacter(PC, ChampionRowName, Team, 1);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::StartGame] Start Game."));

    int32 GameTimerID = FMath::Rand();

    AOSGameState->StartGame();
    SetTimer(GameTimers, BroadcastGameTimerHandles, GameTimerID, [this]() { ActivateSpawnMinion(); }, &AAOSGameMode::BroadcastRemainingTime, LoadedGameData.MinionSpawnInterval, true, LoadedGameData.MinionSpawnTime);
    ActivateCurrencyIncrement();
}

void AAOSGameMode::SpawnCharacter(AAOSPlayerController* PlayerController, const FName& ChampionRowName, ETeamSideBase Team, const int32 PlayerIndex)
{
    // �÷��̾� ���� ������ �����մϴ�.
    FName PlayerStartName = Team == ETeamSideBase::Blue
        ? FName(*FString::Printf(TEXT("Blue%d"), PlayerIndex))
        : FName(*FString::Printf(TEXT("Red%d"), PlayerIndex));

    // ��ȿ�� �÷��̾� ���� ������ �ִ��� Ȯ���մϴ�.
    if (PlayerStarts.Num() == 0 || !PlayerStarts.Contains(PlayerStartName))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid PlayerStart %s."), *PlayerStartName.ToString());
        return;
    }

    UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(GetGameInstance());
    if (!::IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid GameInstance."));
        return;
    }

    // è�Ǿ� ������ ���̺��� ĳ���� �����͸� �����ɴϴ�.
    const FChampionsListRow* CharacterData = GameInstance->GetCampionsListTableRow(ChampionRowName);
    if (!CharacterData || !CharacterData->CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid CharacterIndex or CharacterClass."));
        return;
    }

    // ĳ���͸� �����մϴ�.
    AAOSCharacterBase* Character = GetWorld()->SpawnActor<AAOSCharacterBase>(CharacterData->CharacterClass, PlayerStarts[PlayerStartName]->GetActorTransform());
    if (!::IsValid(Character))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn character."));
        return;
    }

    // GameState�� ��ȿ�ϸ� �÷��̾� ĳ���͸� �߰��մϴ�.
    if (::IsValid(AOSGameState))
    {
        AOSGameState->AddPlayerCharacter(Character, Team);
    }

    // �÷��̾� ��Ʈ�ѷ��� ĳ���͸� �����ϰ� �ϰ�, �ε� ȭ���� �����մϴ�.
    PlayerController->Possess(Character);
    PlayerController->RemoveLoadingScreen();

    // �÷��̾� ĳ���� �ʿ� �߰��մϴ�.
    PlayerCharacterMap.FindOrAdd(PlayerController, Character);
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

void AAOSGameMode::SendLoadedItemsToClients()
{
    if (AOSGameState)
    {
        TArray<FItemInformation> Items;
        GetLoadedItems(Items);
        AOSGameState->SetLoadedItems(Items);
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
            if (!::IsValid(PlayerStateSave))
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

    // ���� ������ Ŭ����
    LoadedMinionData.Empty();

    TArray<FName> RowNames = DataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FMinionDataTableRow* MinionData = DataTable->FindRow<FMinionDataTableRow>(RowName, TEXT(""));
        if (!MinionData)
        {
            UE_LOG(LogTemp, Warning, TEXT("[AAOSGameMode::LoadMinionData] Failed to find row: %s"), *RowName.ToString());
            continue;
        }

        LoadedMinionData.Add(MinionData->MinionType, *MinionData);

        // �̴Ͼ� Ÿ�Ժ��� ĳ���� ���ҽ��� ĳ��
        if (!CachedCharacterResources.Contains(MinionData->MinionType))
        {
            CachedCharacterResources.Add(MinionData->MinionType, CacheCharacterResources(MinionData->ResourcesTable));
        }
    }
}

FCharacterGamePlayDataRow AAOSGameMode::CacheCharacterResources(UDataTable* ResourcesTable)
{
    FCharacterGamePlayDataRow CachedResources;
    FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
    if (DataRow)
    {
        CachedResources = *DataRow;
    }
    return CachedResources;
}

void AAOSGameMode::ActivateCurrencyIncrement()
{
    GetWorldTimerManager().SetTimer(CurrencyIncrementTimerHandle, this, &AAOSGameMode::IncrementPlayerCurrency, 1.0f, true);
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
                PlayerState->SetCurrency(PlayerState->GetCurrency() + LoadedGameData.IncrementCurrencyAmount);
            }
        }
    }
}

void AAOSGameMode::ActivateSpawnMinion()
{
    GetWorldTimerManager().SetTimer(MinionSpawnTimerHandle, [this]()
        {
            SpawnCount++;
            UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::ActivateSpawnMinion] SpawnCount: %d"), SpawnCount);

            // �� ���ο� ���� �̴Ͼ� ���� ���� �α�
            //SpawnMinionsForLane("Top");
            SpawnMinionsForLane("Mid");
           // SpawnMinionsForLane("Bottom");
        }, LoadedGameData.MinionSpawnInterval, true, 0.0f); // 90�ʸ��� �ݺ� ����

    UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::ActivateSpawnMinion] Timer set for minion spawning every 90 seconds."));
}


void AAOSGameMode::SpawnMinionsForLane(const FName& Lane)
{
    const int32 MinionsPerWave = (SpawnCount % LoadedGameData.SuperMinionSpawnInterval == 0) ? LoadedGameData.MinionsPerWave + 1 : LoadedGameData.MinionsPerWave;
    int32 TimerID = FMath::Rand();
    TSharedPtr<int32> CurrentIndex = MakeShareable(new int32(0)); 

    auto TimerCallback = [this, Lane, MinionsPerWave, CurrentIndex, TimerID]()
        {
            if (*CurrentIndex >= MinionsPerWave)
            {
                // ��� �̴Ͼ� ������ �Ϸ�Ǿ����Ƿ� Ÿ�̸Ӹ� ����
                ClearTimer(GameTimers, BroadcastGameTimerHandles, TimerID);
                UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::SpawnMinionsForLane] Completed spawning minions for Lane: %s"), *Lane.ToString());
                return;
            }

            EMinionType MinionType;

            if (SpawnCount % LoadedGameData.SuperMinionSpawnInterval == 0) // 3��° ��ȯ����
            {
                if (*CurrentIndex < LoadedGameData.MinionsPerWave / 2)
                {
                    MinionType = EMinionType::Melee; // ó�� 3������ Melee
                    UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::SpawnMinionsForLane] Spawning Melee Minion at %s, Index: %d"), *Lane.ToString(), *CurrentIndex);
                }
                else if (*CurrentIndex == LoadedGameData.MinionsPerWave / 2)
                {
                    MinionType = EMinionType::Super; // 4��°�� Super Minion
                    UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::SpawnMinionsForLane] Spawning Super Minion at %s, Index: %d"), *Lane.ToString(), *CurrentIndex);
                }
                else
                {
                    MinionType = EMinionType::Ranged; // �������� Ranged
                    UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::SpawnMinionsForLane] Spawning Ranged Minion at %s, Index: %d"), *Lane.ToString(), *CurrentIndex);
                }
            }
            else
            {
                MinionType = (*CurrentIndex < LoadedGameData.MinionsPerWave / 2) ? EMinionType::Melee : EMinionType::Ranged; // ��ҿ��� Melee 3, Ranged 3
                const TCHAR* MinionTypeStr = (*CurrentIndex < 3) ? TEXT("Melee") : TEXT("Ranged");
                UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::SpawnMinionsForLane] Spawning %s Minion at %s, Index: %d"), MinionTypeStr, *Lane.ToString(), *CurrentIndex);
            }

            SpawnMinion(MinionType, Lane.ToString(), ETeamSideBase::Blue);
            SpawnMinion(MinionType, Lane.ToString(), ETeamSideBase::Red);

            (*CurrentIndex)++;
        };

    SetTimer(GameTimers, BroadcastGameTimerHandles, TimerID, TimerCallback, &AAOSGameMode::BroadcastRemainingTime, LoadedGameData.SpawnInterval, true, 0.0f);
}

void AAOSGameMode::SpawnMinion(EMinionType MinionType, const FString& SpawnLine, ETeamSideBase Team)
{
    FName PlayerStartName = Team == ETeamSideBase::Blue ? FName(*("Blue" + SpawnLine)) : FName(*("Red" + SpawnLine));

    // SplinePaths �� �ش� ������ �ִ��� Ȯ��
    if (!SplinePaths.Contains(SpawnLine))
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] SplinePath for line: %s not found."), *SpawnLine);
        return;
    }

    FMinionDataTableRow* MinionDataPtr = LoadedMinionData.Find(MinionType);
    if (!MinionDataPtr)
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] Invalid MinionType: %d"), (int32)MinionType);
        return;
    }

    // PlayerStart �迭�� ��ȿ�� Ȯ��
    if (MinionStarts.Num() == 0 || !MinionStarts.Contains(PlayerStartName))
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] Invalid PlayerStart for name: %s"), *PlayerStartName.ToString());
        return;
    }

    // Minion ���� ����
    AActor* PlayerStartActor = MinionStarts[PlayerStartName];
    if (!PlayerStartActor)
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] PlayerStartActor for name: %s is null."), *PlayerStartName.ToString());
        return;
    }

    FTransform SpawnTransform = PlayerStartActor->GetActorTransform();
    AMinionBase* NewMinion = Cast<AMinionBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, MinionDataPtr->MinionClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
    if (!NewMinion)
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] Failed to spawn Minion of type: %d"), (int32)MinionType);
        return;
    }

    // SkeletalMeshComponent ��ȿ�� Ȯ��
    if (!MinionDataPtr->SkeletalMesh_Down || !MinionDataPtr->SkeletalMesh_Dusk)
    {
        UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] Missing SkeletalMesh for MinionType: %d"), (int32)MinionType);
        return;
    }

    NewMinion->ReplicatedSkeletalMesh = Team == ETeamSideBase::Blue ? MinionDataPtr->SkeletalMesh_Down : MinionDataPtr->SkeletalMesh_Dusk;
    NewMinion->ObjectType = EObjectType::Minion;
    NewMinion->TeamSide = Team;

    NewMinion->ExperienceShareRadius = LoadedGameData.ExperienceShareRadius;
    NewMinion->ShareFactor.Add(1, 1.0f);
    NewMinion->ShareFactor.Add(2, LoadedGameData.ExpShareFactorTwoPlayers);
    NewMinion->ShareFactor.Add(3, LoadedGameData.ExpShareFactorThreePlayers);
    NewMinion->ShareFactor.Add(4, LoadedGameData.ExpShareFactorFourPlayers);
    NewMinion->ShareFactor.Add(5, LoadedGameData.ExpShareFactorFivePlayers);

    NewMinion->SetExpBounty(MinionDataPtr->ExpBounty);
    NewMinion->SetGoldBounty(MinionDataPtr->GoldBounty);

    NewMinion->MaxChaseDistance = LoadedGameData.MaxChaseDistance;

    const FCharacterGamePlayDataRow* CachedResourcesPtr = CachedCharacterResources.Find(MinionType);
    if (CachedResourcesPtr)
    {
        NewMinion->CharacterAnimations = CachedResourcesPtr->GetGamePlayMontagesMap();
        NewMinion->CharacterParticles = CachedResourcesPtr->GetGamePlayParticlesMap();
        NewMinion->CharacterMeshes = CachedResourcesPtr->GetGamePlayMeshesMap();
    }

    // SplineActor ����
    NewMinion->SplineActor = SplinePaths[SpawnLine];

    UGameplayStatics::FinishSpawningActor(NewMinion, SpawnTransform);

    // Minion�� AIController ���ǽ�Ű��
    AMinionAIController* MinionAIController = Cast<AMinionAIController>(NewMinion->GetController());
    if (!MinionAIController)
    {
        MinionAIController = GetWorld()->SpawnActor<AMinionAIController>(AMinionAIController::StaticClass(), SpawnTransform);
        if (!MinionAIController)
        {
            UE_LOG(LogTemp, Error, TEXT("[AAOSGameMode::SpawnMinion] Failed to spawn AIController for MinionType: %d"), (int32)MinionType);
            return;
        }
        MinionAIController->Possess(NewMinion);
    }

    if (MinionAIController)
    {
        MinionAIController->BeginAI(NewMinion);
    }

    UE_LOG(LogTemp, Log, TEXT("[AAOSGameMode::SpawnMinion] Successfully spawned minion of type: %d at lane: %s"), (int32)MinionType, *SpawnLine);
}


void AAOSGameMode::FindPlayerStart()
{
    // �÷��̾� ���� ������ �̸� Ȯ���� �� �ֵ��� �迭�� �����մϴ�.
    constexpr int32 ExpectedPlayerStarts = 12; // �����Ǵ� �÷��̾� ���� ���� ��
    constexpr int32 ExpectedMinionStarts = 6; // �����Ǵ� �̴Ͼ� ���� ���� ��
    PlayerStarts.Reserve(ExpectedPlayerStarts);
    MinionStarts.Reserve(ExpectedMinionStarts);

    // �÷��̾� ���� ������ �ϰ������� �˻��մϴ�.
    TArray<AActor*> PlayerStartActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartActors);

    // �˻��� �÷��̾� ���� ������ ���� ��� �α� ���
    if (PlayerStartActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindPlayerStart: No PlayerStart actors found in the level."));
        return;
    }

    for (AActor* Actor : PlayerStartActors)
    {
        if (!Actor || Actor->Tags.Num() == 0)
        {
            continue; // ��ȿ���� ���� Actor�� Tag�� ���� ��� �ǳʶݴϴ�.
        }

        APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);
        if (!PlayerStart)
        {
            continue; // ĳ������ ������ ��� �ǳʶݴϴ�.
        }

        // ù ��° �±׸� �����ɴϴ�.
        FName Tag = Actor->Tags[0];

        // �±׿� ���� ������ �ʿ� �߰��մϴ�.
        if (Tag == FName("Player"))
        {
            PlayerStarts.Add(PlayerStart->PlayerStartTag, PlayerStart);
        }
        else if (Tag == FName("Minion"))
        {
            MinionStarts.Add(PlayerStart->PlayerStartTag, PlayerStart);
        }
    }
}


void AAOSGameMode::FindSplinePath()
{
    TArray<AActor*> FoundSplineActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundSplineActors);

    for (AActor* SplineActor : FoundSplineActors)
    {
        if (SplineActor->Tags.Num() > 0)
        {
            FName Tag = SplineActor->Tags[0];

            if (Tag == FName("Mid"))
            {
                SplinePaths.Add("Mid", SplineActor);
                UE_LOG(LogTemp, Log, TEXT("Added spline path for Mid lane."));
            }
            else if (Tag == FName("Top"))
            {
                SplinePaths.Add("Top", SplineActor);
                UE_LOG(LogTemp, Log, TEXT("Added spline path for Top lane."));
            }
            else if (Tag == FName("Bottom"))
            {
                SplinePaths.Add("Bottom", SplineActor);
                UE_LOG(LogTemp, Log, TEXT("Added spline path for Bottom lane."));
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Spline path finding completed. Found paths for %d lanes."), SplinePaths.Num());
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
    auto TimerCallback = [this, TimerID, WeakPlayerController = TWeakObjectPtr<AAOSPlayerController>(PlayerController)]()
        {
            if (AAOSPlayerController* ValidPlayerController = WeakPlayerController.Get())
            {
                RespawnCharacter(ValidPlayerController);
                AOSGameState->MulticastBroadcastRespawnTime(TimerID, 0.0f);
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
        // ���ο� ĳ���� ���� ���� �ʿ�
    }
}

float AAOSGameMode::CalculateRespawnTime(AAOSCharacterBase* Character) const
{
    if (!AOSGameState)
    {
        return 5.0f; // �⺻ ������ �ð�
    }

    if (UStatComponent* StatComponent = Character->GetStatComponent())
    {
        float ElapsedTime = AOSGameState->GetElapsedTime();
        int32 CharacterLevel = StatComponent->GetCurrentLevel();

        // ������ �ð��� ����ϴ� ����
        float BaseRespawnTime = 5.0f;
        float TimeFactor = ElapsedTime / 60.0f; // �� ������ ����
        float LevelFactor = CharacterLevel * 1.f; // ���� �� 1�� ����

        return BaseRespawnTime + TimeFactor + LevelFactor;
    }

    return 5.f;
}

void AAOSGameMode::SetTimer(TMap<int32, FTimerHandle>& Timers, TMap<int32, FTimerHandle>& BroadcastTimers, int32 TimerID, TFunction<void()> Callback, void (AAOSGameMode::* BroadcastFunc)(int32, float) const, float Duration, bool bLoop, float FirstDelay)
{
    FTimerHandle& TimerHandle = Timers.FindOrAdd(TimerID);
    FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(Callback);
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, bLoop, FirstDelay);

    // �ֱ������� ���� �ð��� ��ε�ĳ��Ʈ�ϴ� Ÿ�̸� ����
    FTimerHandle& BroadcastTimerHandle = BroadcastTimers.FindOrAdd(TimerID);
    GetWorld()->GetTimerManager().SetTimer(BroadcastTimerHandle, [this, &Timers, TimerID, BroadcastFunc]()
        {
            float RemainingTime = GetTimerRemaining(Timers, TimerID);
            (this->*BroadcastFunc)(TimerID, RemainingTime);
        }, 0.5f, true, 0.0f);
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
    // ���� �ð��� ��ε�ĳ��Ʈ�ϴ� ������ ���⿡ �߰�
}

void AAOSGameMode::BroadcastRemainingRespawnTime(int32 PlayerIndex, float RemainingTime) const
{
    if (!::IsValid(AOSGameState))
    {
        return;
    }

    AOSGameState->MulticastBroadcastRespawnTime(PlayerIndex, RemainingTime);
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
    if (!::IsValid(Character))
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
    if (!::IsValid(Character))
    {
        return;
    }

    UStatComponent* StatComopnent = Character->GetStatComponent();
    if (!StatComopnent)
    {
        return;
    }

    int32 CurrentExp = StatComopnent->GetCurrentEXP();
    StatComopnent->ModifyCurrentEXP(Amount);
}
