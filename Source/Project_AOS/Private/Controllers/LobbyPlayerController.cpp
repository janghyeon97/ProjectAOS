// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/LobbyPlayerController.h"
#include "Game/AOSGameInstance.h"
#include "Game/LobbyPlayerState.h"
#include "Game/LobbyGameState.h"
#include "Game/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LobbyUI.h"
#include "UI/UW_ChatWindow.h"
#include "UI/UW_Button.h"
#include "UI/ChampionSelectionUI.h"
#include "UI/UW_ChampionListEntry.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ALobbyPlayerController::ALobbyPlayerController()
{
	AOSGameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	ChatWindow = nullptr;
	bIsHostPlayer = false;
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerState);
	LobbyGameState = Cast<ALobbyGameState>(UGameplayStatics::GetGameState(GetWorld()));

	if (!LobbyGameState)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::BeginPlay] LobbyGameState is not valid."));
		return;
	}

	if (HasAuthority())
	{
		LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (LobbyGameMode)
		{
			LobbyGameMode->OnSelectionTimerChanged.AddDynamic(this, &ThisClass::UpdateBanPickTime_Client);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::BeginPlay] LobbyGameMode is not valid."));
		}
	}
	if (IsLocalPlayerController())
	{
		if (LobbyUIClass)
		{
			LobbyUIInstance = CreateWidget<ULobbyUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), LobbyUIClass);
			if (LobbyUIInstance)
			{
				LobbyUIInstance->AddToViewport();

				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(LobbyUIInstance->GetCachedWidget());
				SetInputMode(Mode);

				bShowMouseCursor = true;
			}
		}
	}
}

void ALobbyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bChangedDraftTime)
	{
		if (DraftTime <= 0)
		{
			bChangedDraftTime = false;
			return;
		}

		DraftTime = FMath::FInterpTo(DraftTime, CurrentDraftTime, DeltaSeconds, 15.f);
		if (ChampionSelectUIInstance)
		{
			ChampionSelectUIInstance->OnBanPickTimeChanged(DraftTime, MaxDraftTime);
		}
	}
}

void ALobbyPlayerController::BindChatWindow(UUW_ChatWindow* Widget)
{
	ChatWindow = Widget;
}

void ALobbyPlayerController::ShowLobbyUI()
{
	if (LobbyUIInstance)
	{
		LobbyUIInstance->AddToViewport();
	}
	else
	{
		LobbyUIInstance = CreateWidget<ULobbyUI>(this, LobbyUIClass);
		if (LobbyUIInstance)
		{
			LobbyUIInstance->AddToViewport();
		}
	}
}

void ALobbyPlayerController::ShowLoadingScreen_Client_Implementation()
{
	if (LoadingScreenInstance)
	{
		LoadingScreenInstance->AddToViewport();
	}
	else
	{
		LoadingScreenInstance = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		if (LoadingScreenInstance)
		{
			LoadingScreenInstance->AddToViewport();
		}
	}
}

void ALobbyPlayerController::ShowChampionSelectUI_Server_Implementation()
{
	if (!LobbyGameState)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::ShowChampionSelectUI_Server] LobbyGameState is not valid."));
		return;
	}

	const TArray<TObjectPtr<APlayerState>>& AllPlayers = LobbyGameState->PlayerArray;
	for (auto& CurrentPlayer : AllPlayers)
	{
		ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(CurrentPlayer->GetPlayerController());
		if (::IsValid(LobbyPlayerController))
		{
			LobbyPlayerController->ShowChampionSelectUI_Client();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::ShowChampionSelectUI_Server] Invalid LobbyPlayerController for player %s."), *CurrentPlayer->GetPlayerName());
		}
	}

	if (::IsValid(LobbyGameMode))
	{
		LobbyGameMode->MatchState = EMatchState::Picking;
		LobbyGameMode->StartBanPick();
	}
}

void ALobbyPlayerController::ShowChampionSelectUI_Client_Implementation()
{
	if (::IsValid(ChampionSelectUIInstance))
	{
		ChampionSelectUIInstance->AddToViewport();
	}
	else
	{
		if (ChampionSelectUIClass == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::ShowChampionSelectUI_Client] ChampionSelectUIClass is not set."));
			return;
		}

		ChampionSelectUIInstance = CreateWidget<UChampionSelectionUI>(this, ChampionSelectUIClass);
		if (::IsValid(ChampionSelectUIInstance))
		{
			ChampionSelectUIInstance->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::ShowChampionSelectUI_Client] Failed to create ChampionSelectUIInstance."));
		}
	}
}

void ALobbyPlayerController::SetHostPlayer_Client_Implementation(bool bIsHost)
{
	bIsHostPlayer = bIsHost;
	if (LobbyUIInstance)
	{
		UUW_Button* GameStartButton = LobbyUIInstance->GetGameStartButton();
		if (GameStartButton)
		{
			GameStartButton->SetIsEnabled(bIsHost);
		}
	}
}

void ALobbyPlayerController::UpdateChatLog_Server_Implementation(const FString& Message, AController* EventInstigator)
{
	if (::IsValid(LobbyGameMode) == false || ::IsValid(LobbyGameState) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::UpdateChatLog_Server] LobbyGameMode or LobbyGameState is not valid."));
		return;
	}

	ALobbyPlayerState* SenderPlayerState = EventInstigator->GetPlayerState<ALobbyPlayerState>();
	if (!SenderPlayerState)
	{
		return;
	}

	FString SenderName = SenderPlayerState->GetPlayerName();

	TArray<TObjectPtr<ALobbyPlayerState>> Players;
	if (LobbyGameMode->MatchState == EMatchState::Waiting)
	{
		for (APlayerState* CurrentPlayer  : LobbyGameState->PlayerArray)
		{
			if (ALobbyPlayerState* NewPlayerState = Cast<ALobbyPlayerState>(CurrentPlayer))
			{
				Players.Add(NewPlayerState);
			}
		}
	}
	else if (LobbyGameMode->MatchState == EMatchState::Picking)
	{
		if (SenderPlayerState->TeamSide == ETeamSideBase::Blue)
		{
			Players = LobbyGameState->BlueTeamPlayers;
		}
		else if (SenderPlayerState->TeamSide == ETeamSideBase::Red)
		{
			Players = LobbyGameState->RedTeamPlayers;
		}
	}

	for (APlayerState* CurrentPlayer  : Players)
	{
		ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(CurrentPlayer->GetPlayerController());
		if (LobbyPlayerController)
		{
			FString Result = (EventInstigator == LobbyPlayerController) ? FString::Printf(TEXT("<Yellow>%s</>: %s"), *SenderName, *Message) : FString::Printf(TEXT("<White>%s</>: %s"), *SenderName, *Message);
			LobbyPlayerController->UpdateChatLog_Client(Result);
		}
	}
}

void ALobbyPlayerController::UpdateChatLog_Client_Implementation(const FString& Text)
{
	if (::IsValid(ChatWindow) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] ALobbyPlayerController::UpdateChatLog_Client ChatWindow is not valid!"));
		return;
	}


	UE_LOG(LogTemp, Log, TEXT("[Client] %s received message: %s"), *PlayerState->GetPlayerName(), *Text);
	ChatWindow->UpdateMessageLog(Text);
}

void ALobbyPlayerController::UpdatePlayerSelection_Server_Implementation(ETeamSideBase Team, int32 PlayerIndex, const FString& InPlayerName, int32 ChampionIndex, FLinearColor Color, bool bShowChampionDetails)
{
	if (::IsValid(LobbyGameMode) == false || ::IsValid(LobbyGameState) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::UpdateTeamSelectInfo_Server] LobbyGameMode or LobbyGameState is not valid."));
		return;
	}

	if (Team == ETeamSideBase::Type)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::UpdateTeamSelectInfo_Server] Team 값이 유효하지 않습니다: Type"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Server] ALobbyPlayerController::UpdateTeamSelectInfo_Server :: %d, %s, %d, %s, %s"),
		PlayerIndex, *InPlayerName, ChampionIndex, *Color.ToString(), bShowChampionDetails ? TEXT("True") : TEXT("False"));


	const TArray<TObjectPtr<APlayerState>>& AllPlayers = LobbyGameState->PlayerArray;
	for (auto& CurrentPlayer : AllPlayers)
	{
		ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(CurrentPlayer->GetPlayerController());
		if (LobbyPlayerController)
		{
			LobbyPlayerController->UpdatePlayerSelection_Client(Team, PlayerIndex, InPlayerName, ChampionIndex, Color, bShowChampionDetails);
		}
	}
}

void ALobbyPlayerController::UpdatePlayerSelection_Client_Implementation(ETeamSideBase Team, int32 PlayerIndex, const FString& InPlayerName, int32 ChampionIndex, FLinearColor Color, bool bShowChampionDetails)
{
	if (::IsValid(AOSGameInstance) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::UpdatePlayerSelection_Client] %s Player's AOSGameInstance is not valid."), *PlayerState->GetPlayerName());
		return;
	}

	if (::IsValid(ChampionSelectUIInstance) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::UpdatePlayerSelection_Client] %s Player's ChampionSelectUIInstance is not valid."), *PlayerState->GetPlayerName());
		return;
	}

	UTexture* ChampionImageTexture	= AOSGameInstance->GetCampionsListTableRow(ChampionIndex)->ChampionImage;
	FString ChampionNameString		= AOSGameInstance->GetCampionsListTableRow(ChampionIndex)->ChampionName;
	FString ChampionPositionString	= AOSGameInstance->GetCampionsListTableRow(ChampionIndex)->Position;

	UE_LOG(LogTemp, Log, TEXT("[Client] ALobbyPlayerController::UpdateTeamSelectInfo_Client :: %d, %s, %s, %s, %s, %s"),
		PlayerIndex, *InPlayerName, *ChampionNameString, *ChampionPositionString, *Color.ToString(), bShowChampionDetails ? TEXT("True") : TEXT("False"));

	ChampionSelectUIInstance->UpdatePlayerSelection(Team, PlayerIndex, InPlayerName, ChampionImageTexture, ChampionNameString, ChampionPositionString, Color, bShowChampionDetails);
}

void ALobbyPlayerController::UpdateBanPickTime_Client_Implementation(float InCurrentDraftTime, float InMaxDraftTime)
{
	if (::IsValid(ChampionSelectUIInstance) == false)
	{
		return;
	}

	if (InCurrentDraftTime >= InMaxDraftTime)
	{
		DraftTime = InCurrentDraftTime;
	}
	else
	{
		bChangedDraftTime = true;
	}

	CurrentDraftTime = InCurrentDraftTime;
	MaxDraftTime = InMaxDraftTime;
}
