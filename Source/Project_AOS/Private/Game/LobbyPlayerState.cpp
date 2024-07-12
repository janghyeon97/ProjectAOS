// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	TeamSide = ETeamSideBase::Type;
	PlayerIndex = -1;
	SelectedChampionIndex = -1;
	PlayerUniqueID = FString();
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
}

void ALobbyPlayerState::UpdateSelectedChampion_Server_Implementation(int32 Index)
{
	SelectedChampionIndex = Index;
}

FString ALobbyPlayerState::GetPlayerUniqueIdString() const
{
	if (GetUniqueId().IsValid() && GetUniqueId()->IsValid())
	{
		return GetUniqueId()->ToString();
	}
	return FString();
}

