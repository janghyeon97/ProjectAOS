#include "UI/ChampionSelectionUI.h"
#include "UI/UW_ChampionListEntry.h"
#include "UI/UW_ChampionSelection.h"
#include "UI/UW_EditableText.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/WrapBox.h"
#include "Components/Border.h"
#include "Controllers/LobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AOSGameInstance.h"
#include "Game/LobbyPlayerState.h"
#include "Game/LobbyGameState.h"


void UChampionSelectionUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BlueTeamSelection)
	{
		BlueTeamWidgets = BlueTeamSelection->GetAllChildren();
	}

	if (RedTeamSelection)
	{
		RedTeamWidgets = RedTeamSelection->GetAllChildren();
	}
}

void UChampionSelectionUI::NativeConstruct()
{
	Super::NativeConstruct();

	AOSGameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	LobbyGameState = Cast<ALobbyGameState>(UGameplayStatics::GetGameState(GetWorld()));

	if (!AOSGameInstance.IsValid() || !LobbyGameState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[UChampionSelectionUI::NativeConstruct] AOSGameInstance or LobbyGameState is not valid."));
		return;
	}

	InitializePlayerList();
	InitializeChampionList();
}

void UChampionSelectionUI::InitializeChampionList()
{
	if (!AOSGameInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[UChampionSelectionUI::InitializeChampionList] AOSGameInstance is not valid."));
		return;
	}

	const auto& RowMap = AOSGameInstance->GetCampionsListTable()->GetRowMap();
	for (int32 i = 1; i <= RowMap.Num(); i++)
	{
		const auto* Row = AOSGameInstance->GetCampionsListTableRow(i);
		if (Row)
		{
			AddChampionListEntry(Row->Index, Row->ChampionName, Row->ChampionImage);
		}
	}
}

void UChampionSelectionUI::InitializePlayerList()
{
	if (!LobbyGameState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[UChampionSelectionUI::InitializePlayerList] LobbyGameState is not valid."));
		return;
	}

	const TArray<TObjectPtr<APlayerState>>& Players = LobbyGameState->PlayerArray;

	for (APlayerState* Player : Players)
	{
		ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(Player);
		if (LobbyPlayerState)
		{
			AddPlayerSelection(LobbyPlayerState->TeamSide, Player->GetPlayerName());
		}
	}
}

void UChampionSelectionUI::AddChampionListEntry(int32 Index, const FString& InChampionName, UTexture* Texture)
{
	if (!ChampionListBox || !ChampionListEntryClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[UChampionSelectionUI::AddChampionListEntry] ChampionListBox or ChampionListEntryClass is not valid."));
		return;
	}

	UUW_ChampionListEntry* ChampionEntry = CreateWidget<UUW_ChampionListEntry>(ChampionListBox, ChampionListEntryClass);
	if (ChampionEntry)
	{
		ChampionEntry->InitializeListEntry();
		ChampionEntry->UpdateChampionIndex(Index);
		ChampionEntry->UpdateChampionNameText(InChampionName);
		ChampionEntry->UpdateChampionImage(Texture);
		ChampionListBox->AddChild(ChampionEntry);
	}
}

void UChampionSelectionUI::AddPlayerSelection(ETeamSideBase Team, const FString& InPlayerName)
{
	switch (Team)
	{
	case ETeamSideBase::Type:
		UE_LOG(LogTemp, Error, TEXT("[UChampionSelectionUI::AddPlayerSelection] Team 값이 유효하지 않습니다: Type"));
		break;
	case ETeamSideBase::Blue:
		AddPlayerSelectionUI(BlueTeamPlayers, BlueTeamWidgets, BlueTeamCurrentIndex, InPlayerName);
		break;
	case ETeamSideBase::Red:
		AddPlayerSelectionUI(RedTeamPlayers, RedTeamWidgets, RedTeamCurrentIndex, InPlayerName);
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[UChampionSelectionUI::AddPlayerSelection] 알 수 없는 Team 값: %d"), static_cast<int32>(Team));
		break;
	}
}

void UChampionSelectionUI::AddPlayerSelectionUI(TArray<TObjectPtr<UUW_ChampionSelection>>& TeamPlayers, TArray<TObjectPtr<UWidget>>& TeamWidgets, uint8& CurrentIndex, const FString& InPlayerName)
{
	uint8 Index = FMath::Clamp<uint8>(CurrentIndex + 2, 0, TeamWidgets.Num() - 1);

	for (int i = CurrentIndex; i <= Index; i++)
	{
		if (UWidget* Widget = TeamWidgets[i].Get())
		{
			Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			UUW_ChampionSelection* ChampionSelection = Cast<UUW_ChampionSelection>(Widget);
			if (ChampionSelection)
			{
				ChampionSelection->UpdatePlayerNameText(InPlayerName);
				TeamPlayers.Add(ChampionSelection);
			}
		}
	}

	CurrentIndex = Index;
}

void UChampionSelectionUI::UpdatePlayerSelection(ETeamSideBase Team, const int32 PlayerIndex, const FString& InPlayerName, UTexture* Texture, const FString& InChampionName, const FString& InChampionPosition, FLinearColor Color, bool bShowChampionDetails)
{
	switch (Team)
	{
	case ETeamSideBase::Type:
		break;
	case ETeamSideBase::Blue:
		UpdatePlayerSelectionUI(BlueTeamPlayers, PlayerIndex, InPlayerName, Texture, InChampionName, InChampionPosition, Color, bShowChampionDetails);
		break;
	case ETeamSideBase::Red:
		UpdatePlayerSelectionUI(RedTeamPlayers, PlayerIndex, InPlayerName, Texture, InChampionName, InChampionPosition, Color, bShowChampionDetails);
		break;
	}
}

void UChampionSelectionUI::UpdatePlayerSelectionUI(TArray<TObjectPtr<UUW_ChampionSelection>>& TeamPlayers, const int32 PlayerIndex, const FString& InPlayerName, UTexture* Texture, const FString& InChampionName, const FString& InChampionPosition, FLinearColor Color, bool bShowChampionDetails)
{
	if (!TeamPlayers.IsValidIndex(PlayerIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("[UChampionSelectionUI::UpdatePlayerSelectionUI] Can't access TeamPlayer array at index %d"), PlayerIndex);
		return;
	}

	if (UUW_ChampionSelection* ChampionSelection = TeamPlayers[PlayerIndex].Get())
	{
		ChampionSelection->UpdateCampionImage(Texture);
		ChampionSelection->UpdateChampionNameText(InChampionName);
		ChampionSelection->UpdateChampionPositionText(InChampionPosition);
		ChampionSelection->UpdatePlayerNameText(InPlayerName);
		ChampionSelection->UpdateBorderImageColor(Color);
		ChampionSelection->UpdateChampionNameColor(Color);
		ChampionSelection->UpdateChampionPositionColor(Color);
		ChampionSelection->UpdatePlayerNameColor(Color);
		ChampionSelection->SetChampionInfoVisibility(bShowChampionDetails);
	}
}

void UChampionSelectionUI::UpdateInfomationText(const FString& String)
{
	if (InfomationText)
	{
		InfomationText->SetText(FText::FromString(String));
	}
}

void UChampionSelectionUI::OnBanPickTimeChanged(float CurrentTime, float MaxTime)
{
	FString TimerString = FString::Printf(TEXT("%d"), FMath::CeilToInt(CurrentTime));
	if (TimerText)
	{
		TimerText->SetText(FText::FromString(TimerString));
	}
	if (BlueProgressBar)
	{
		BlueProgressBar->SetPercent(CurrentTime / MaxTime);
	}
	if (RedProgressBar)
	{
		RedProgressBar->SetPercent(CurrentTime / MaxTime);
	}
}
