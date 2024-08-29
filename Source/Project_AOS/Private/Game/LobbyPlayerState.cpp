// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	TeamSide = ETeamSideBase::Type;
	PlayerIndex = -1;
	SelectedChampionIndex = -1;
	SelectedChampionName = NAME_None;
	PlayerUniqueID = NAME_None;
}

void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		
	}
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamSide);
	DOREPLIFETIME(ThisClass, PlayerIndex);
	DOREPLIFETIME(ThisClass, SelectedChampionIndex);
	DOREPLIFETIME(ThisClass, PlayerUniqueID); 
	DOREPLIFETIME(ThisClass, SelectedChampionName);
}

void ALobbyPlayerState::UpdateSelectedChampion_Server_Implementation(const int32 Index, const FName& InName)
{
	SelectedChampionIndex = Index;
	SelectedChampionName = InName;
}

FName ALobbyPlayerState::GetPlayerUniqueIdString() const
{
	if (GetUniqueId().IsValid() && GetUniqueId()->IsValid())
	{
		return FName(*GetUniqueId()->ToString());
	}
	return FName();
}

