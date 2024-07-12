// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/JoinMenuUI.h"
#include "Components/ListView.h"
#include "Components/Button.h"
#include "Structs/SessionInfomation.h"
#include "Plugins/MultiplaySessionSubsystem.h"


void UJoinMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	UMultiplaySessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();

	CancleButton->OnClicked.AddDynamic(this, &UJoinMenuUI::RemoveJoinMenu);
	RefreshButton->OnClicked.AddDynamic(this, &UJoinMenuUI::FindSession);
	SessionSubsystem->OnFindSessionsCompleteEvent.AddUObject(this, &UJoinMenuUI::AddEntryServerList);
}

void UJoinMenuUI::FindSession()
{
	ServerList->ClearListItems();

	UMultiplaySessionSubsystem* SessionSystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
	if (::IsValid(SessionSystem))
	{
		SessionSystem->FindSessions(10, false);
	}
}

void UJoinMenuUI::AddEntryServerList(const TArray<USessionInfomation*>& SessionResults, bool Successful)
{
	for (auto& Session : SessionResults)
	{
		ServerList->AddItem(Session);
	}
}

void UJoinMenuUI::RemoveJoinMenu()
{
	RemoveFromParent();
}