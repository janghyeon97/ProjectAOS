// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LobbyGameMode.h"
#include "Game/AOSGameInstance.h"
#include "Game/LobbyGameState.h"
#include "Game/LobbyPlayerState.h"
#include "Game/PlayerStateSave.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Plugins/MultiplaySessionSubsystem.h"
#include "Controllers/LobbyPlayerController.h"
#include "Characters/AuroraCharacter.h"
#include "Kismet/GameplayStatics.h"

ALobbyGameMode::ALobbyGameMode()
{
	bIsFirstConnectedPlayer = true;

	NumberOfConnectedPlayers = 0;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(BanPickTimer);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//FString PharseNickname = UGameplayStatics::ParseOption(OptionsString, FString(TEXT("Nickname")));

	ALobbyPlayerController* NewPlayerController = Cast<ALobbyPlayerController>(NewPlayer);
	if (::IsValid(NewPlayerController))
	{
		/*FUniqueNetIdRepl UniqueNetIDRepl;
		if (NewPlayerController->IsLocalPlayerController())
		{
			ULocalPlayer* LocalPlayer = NewPlayerController->GetLocalPlayer();
			if (::IsValid(LocalPlayer))
			{
				UniqueNetIDRepl = LocalPlayer->GetPreferredUniqueNetId();
			}
			else
			{
				UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(NewPlayerController->Player);
				check(::IsValid(RemoteNetConnectionRef));
				UniqueNetIDRepl = RemoteNetConnectionRef->PlayerId;
			}
		}

		TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIDRepl.GetUniqueNetId();
		check(UniqueNetId != nullptr)
		const IOnlineSubsystem* SubsytemRef = Online::GetSubsystem(NewPlayerController->GetWorld());
		IOnlineSessionPtr SessionRef = SubsytemRef->GetSessionInterface();

		bool bRegistrationSuccess = SessionRef->RegisterPlayer(FName("MainSession"), *UniqueNetId, false);
		if (bRegistrationSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("Registration Successful"));
		}*/


		/*
			로비에 처음 들어온 플레이어를 호스트 플레이어로 설정합니다.
			호스트 플레이어는 게임 시작 버튼이 활성화 됩니다.
		*/ 
		if (bIsFirstConnectedPlayer)
		{
			bIsFirstConnectedPlayer = false;
			NewPlayerController->SetHostPlayer_Client(true);
		}

		if (BlueTeamPlayerControllers.Num() <= RedTeamPlayerControllers.Num())
		{
			ALobbyPlayerState* LobbyPlayerState = NewPlayerController->GetPlayerState<ALobbyPlayerState>();
			if (::IsValid(LobbyPlayerState))
			{
				LobbyPlayerState->TeamSide = ETeamSideBase::Blue;
				LobbyPlayerState->PlayerIndex = BlueTeamPlayerControllers.Num();
				LobbyPlayerState->PlayerUniqueID = LobbyPlayerState->GetPlayerUniqueIdString();

				BlueTeamPlayerControllers.AddUnique(NewPlayerController);

				ALobbyGameState* LobbyGameState = Cast<ALobbyGameState>(GameState);
				if (::IsValid(LobbyGameState))
				{
					LobbyGameState->BlueTeamPlayers.Add(LobbyPlayerState);
					UE_LOG(LogTemp, Warning, TEXT("[Server] Blue Side New Player Login : %s"), *LobbyPlayerState->GetPlayerName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[Server] LobbyGameState is not vaild"));
				}
			}
		}
		else
		{
			ALobbyPlayerState* LobbyPlayerState = NewPlayerController->GetPlayerState<ALobbyPlayerState>();
			if (::IsValid(LobbyPlayerState))
			{
				LobbyPlayerState->TeamSide = ETeamSideBase::Red;
				LobbyPlayerState->PlayerIndex = RedTeamPlayerControllers.Num() + 5;
				LobbyPlayerState->PlayerUniqueID = LobbyPlayerState->GetPlayerUniqueIdString();

				RedTeamPlayerControllers.AddUnique(NewPlayerController);

				ALobbyGameState* LobbyGameState = Cast<ALobbyGameState>(GameState);
				if (::IsValid(LobbyGameState))
				{
					LobbyGameState->RedTeamPlayers.Add(LobbyPlayerState);
					UE_LOG(LogTemp, Warning, TEXT("[Server] Red Side New Player Login : %s"), *LobbyPlayerState->GetPlayerName());
				}
			}
		}

		NumberOfConnectedPlayers++;
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ALobbyPlayerController* ExitingPlayerController = Cast<ALobbyPlayerController>(Exiting);
	if (::IsValid(ExitingPlayerController))
	{
		ALobbyPlayerState* LobbyPlayerState = ExitingPlayerController->GetPlayerState<ALobbyPlayerState>();
		if (::IsValid(LobbyPlayerState))
		{
			ALobbyGameState* LobbyGameState = Cast<ALobbyGameState>(GameState);
			if (::IsValid(LobbyGameState))
			{
				if (INDEX_NONE != LobbyGameState->BlueTeamPlayers.Find(LobbyPlayerState) || INDEX_NONE != LobbyGameState->RedTeamPlayers.Find(LobbyPlayerState))
				{
					LobbyGameState->BlueTeamPlayers.Remove(LobbyPlayerState);
					LobbyGameState->BlueTeamPlayers.Remove(LobbyPlayerState);
				}
			}
		}

		if (INDEX_NONE != BlueTeamPlayerControllers.Find(ExitingPlayerController) || INDEX_NONE != RedTeamPlayerControllers.Find(ExitingPlayerController))
		{
			BlueTeamPlayerControllers.Remove(ExitingPlayerController);
			RedTeamPlayerControllers.Remove(ExitingPlayerController);
		}

		NumberOfConnectedPlayers--;
	}
}

void ALobbyGameMode::StartBanPick()
{
	if (MatchState == EMatchState::Picking)
	{
		CurruentBanPickTime = BanPickTime;

		if (OnSelectionTimerChanged.IsBound())
		{
			OnSelectionTimerChanged.Broadcast(CurruentBanPickTime, BanPickTime);
		}

		uint8 NumberOfPlayers = BlueTeamPlayerControllers.Num() + RedTeamPlayerControllers.Num();
		UE_LOG(LogTemp, Log, TEXT("[Server] ALobbyGameMode - Start BanPick | NumberOfPlayers: %d"), NumberOfPlayers);

		GetWorld()->GetTimerManager().SetTimer(BanPickTimer, this, &ALobbyGameMode::UpdateBanPickTime, 0.5f, true, 0.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ALobbyGameMode - Can not Start Ban Pick, MatchState is not Picking State."));
	}
}

void ALobbyGameMode::UpdateBanPickTime()
{
	CurruentBanPickTime -= 0.5f;

	if (CurruentBanPickTime <= 60 && OnSelectionTimerChanged.IsBound())
	{
		OnSelectionTimerChanged.Broadcast(CurruentBanPickTime, BanPickTime);
	}

	if (CurruentBanPickTime <= -1)
	{
		GetWorld()->GetTimerManager().ClearTimer(BanPickTimer);
		GetWorld()->GetTimerManager().SetTimer(BanPickEndTimer, this, &ALobbyGameMode::EndBanPick, 2.0f, false);
	}
}

void ALobbyGameMode::EndBanPick()
{
	SavePlayerData();
	SaveGameData();
}

void ALobbyGameMode::SavePlayerData()
{
	for (auto& PlayerController : BlueTeamPlayerControllers)
	{
		ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerController->PlayerState);
		if (::IsValid(LobbyPlayerState))
		{
			UPlayerStateSave* PlayerStateSave = NewObject<UPlayerStateSave>();
			PlayerStateSave->TeamSide = LobbyPlayerState->TeamSide;
			PlayerStateSave->SelectedChampionIndex = LobbyPlayerState->SelectedChampionIndex;
			PlayerStateSave->SelectedChampionName = LobbyPlayerState->SelectedChampionName;
			PlayerStateSave->PlayerIndex = LobbyPlayerState->PlayerIndex;
			PlayerStateSave->PlayerUniqueID = LobbyPlayerState->PlayerUniqueID;

			FName SaveSlotName = LobbyPlayerState->GetPlayerUniqueIdString();
			UE_LOG(LogTemp, Warning, TEXT("[ALobbyGameMode::SavePlayerData] Save PlayerDate - %s %d %d"), *SaveSlotName.ToString(), PlayerStateSave->PlayerIndex, PlayerStateSave->SelectedChampionIndex);
			if (true == UGameplayStatics::SaveGameToSlot(PlayerStateSave, SaveSlotName.ToString(), 0))
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Saved.")));
			}
		}
	}

	for (auto& PlayerController : RedTeamPlayerControllers)
	{
		ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerController->PlayerState);
		if (::IsValid(LobbyPlayerState))
		{
			UPlayerStateSave* PlayerStateSave = NewObject<UPlayerStateSave>();
			PlayerStateSave->TeamSide = LobbyPlayerState->TeamSide;
			PlayerStateSave->SelectedChampionIndex = LobbyPlayerState->SelectedChampionIndex;
			PlayerStateSave->SelectedChampionName = LobbyPlayerState->SelectedChampionName;
			PlayerStateSave->PlayerIndex = LobbyPlayerState->PlayerIndex;
			PlayerStateSave->PlayerUniqueID = LobbyPlayerState->PlayerUniqueID;

			const FName SaveSlotName = LobbyPlayerState->GetPlayerUniqueIdString();
			UE_LOG(LogTemp, Warning, TEXT("[ALobbyGameMode::SavePlayerData] Save PlayerDate - %s %d %d"), *SaveSlotName.ToString(), PlayerStateSave->PlayerIndex, PlayerStateSave->SelectedChampionIndex);
			if (true == UGameplayStatics::SaveGameToSlot(PlayerStateSave, SaveSlotName.ToString(), 0))
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Saved.")));
			}
		}
	}
}

void ALobbyGameMode::SaveGameData()
{
	UAOSGameInstance* AOSGameInstance = Cast<UAOSGameInstance>(GetGameInstance());
	if (::IsValid(AOSGameInstance))
	{
		AOSGameInstance->NumberOfPlayer = NumberOfConnectedPlayers;
		UE_LOG(LogTemp, Warning, TEXT("[ALobbyGameMode::SaveGameData] Number of player %d"), NumberOfConnectedPlayers);
	}

	ChangeLevel();
}

void ALobbyGameMode::ChangeLevel()
{
	GetWorld()->GetTimerManager().SetTimer(
		BanPickTimer,
		FTimerDelegate::CreateLambda([&]()
			{
				UMultiplaySessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
				if (::IsValid(SessionSubsystem))
				{
					SessionSubsystem->TryServerTravel(L"/Game/ProjectAOS/Level/Arena");
				}
				GetWorld()->GetTimerManager().ClearTimer(BanPickTimer);
			}
		),
		0.1f,
		false,
		1.0f
	);
}

FString ALobbyGameMode::GeneratePlayerUniqueID()
{
	FGuid NewGUID = FGuid::NewGuid();
	return NewGUID.ToString(EGuidFormats::Digits);
}