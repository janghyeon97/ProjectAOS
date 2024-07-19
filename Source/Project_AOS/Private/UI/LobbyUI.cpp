// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LobbyUI.h"
#include "UI/UW_Button.h"
#include "UI/UW_EditableText.h"
#include "UI/UW_LobbyPlayerInfomation.h"
#include "Kismet/GameplayStatics.h"
#include "Game/LobbyGameState.h"
#include "Game/LobbyPlayerState.h"
#include "Controllers/LobbyPlayerController.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/StackBox.h"
#include "Components/Button.h"


void ULobbyUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TArray<UWidget*> widgets;
	
	widgets = BlueTeamStackBox->GetAllChildren();
	for (auto& widget : widgets)
	{
		UWidgetSwitcher* PlayerInfo = Cast<UWidgetSwitcher>(widget);
		if (::IsValid(PlayerInfo))
		{
			BlueTeamSwitcher.AddUnique(PlayerInfo);
		}
	}

	widgets = RedTeamStackBox->GetAllChildren();
	for (auto& widget : widgets)
	{
		UWidgetSwitcher* PlayerInfo = Cast<UWidgetSwitcher>(widget);
		if (::IsValid(PlayerInfo))
		{
			RedTeamSwitcher.AddUnique(PlayerInfo);
		}
	}
}

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	LobbyGameState = Cast<ALobbyGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (::IsValid(LobbyGameState))
	{
		LobbyGameState->OnConnectedPlayerReplicated.AddUObject(this, &ThisClass::UpdateLobby);
	}
	
	LobbyPlayerController = Cast<ALobbyPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	GameStartButton->Button->OnClicked.AddDynamic(this, &ULobbyUI::OnGameStartButtonClicked);

	if (LobbyPlayerController->GetIsHostPlayer())
	{
		GameStartButton->SetIsEnabled(true);
		
	}
	else
	{
		GameStartButton->SetIsEnabled(false);
	}

	UpdateLobby();
}

void ULobbyUI::InitializeSwitcher()
{
	// 블루팀 스위처 초기화
	for (auto& switcher : BlueTeamSwitcher)
	{
		if (::IsValid(switcher))
		{
			switcher->SetActiveWidgetIndex(0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ULobbyUI::InitializeSwitcher] Invalid BlueTeamSwitcher detected."));
		}
	}

	// 레드팀 스위처 초기화
	for (auto& switcher : RedTeamSwitcher)
	{
		if (::IsValid(switcher))
		{
			switcher->SetActiveWidgetIndex(0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ULobbyUI::InitializeSwitcher] Invalid RedTeamSwitcher detected."));
		}
	}
}


void ULobbyUI::UpdateLobby()
{
	InitializeSwitcher();

	TArray<TObjectPtr<APlayerState>> Players = LobbyGameState->PlayerArray;

	if (Players.Num() > 0)
	{
		// 모든 플레이어 상태를 확인하여 팀별로 위젯을 업데이트합니다.
		for (auto& Player : Players)
		{
			ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(Player);
			if (::IsValid(LobbyPlayerState))
			{
				int32 Index = (LobbyPlayerState->TeamSide == ETeamSideBase::Blue) ? LobbyPlayerState->PlayerIndex : LobbyPlayerState->PlayerIndex - 5;
				UUW_LobbyPlayerInfomation* Widget = nullptr;
				TArray<UWidgetSwitcher*>& TeamSwitcher = (LobbyPlayerState->TeamSide == ETeamSideBase::Blue) ? BlueTeamSwitcher : RedTeamSwitcher;

				if (TeamSwitcher.IsValidIndex(Index) && TeamSwitcher[Index]->GetChildrenCount() > 1)
				{
					Widget = Cast<UUW_LobbyPlayerInfomation>(TeamSwitcher[Index]->GetChildAt(1));
					if (::IsValid(Widget))
					{
						Widget->UpdatePlayerNameText(Player->GetPlayerName());
						TeamSwitcher[Index]->SetActiveWidgetIndex(1);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("[ULobbyUI::UpdateLobby] Invalid widget at index %d for team %s."),
							Index,
							(LobbyPlayerState->TeamSide == ETeamSideBase::Blue) ? TEXT("Blue") : TEXT("Red"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[ULobbyUI::UpdateLobby] Invalid index %d or insufficient children for team %s."),
						LobbyPlayerState->PlayerIndex,
						(LobbyPlayerState->TeamSide == ETeamSideBase::Blue) ? TEXT("Blue") : TEXT("Red"));
				}
			}
		}
	}
}

void ULobbyUI::OnGameStartButtonClicked()
{	
	LobbyPlayerController->ShowChampionSelectUI_Server();
}