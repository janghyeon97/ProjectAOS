// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AOSGameState.h"
#include "Characters/AOSCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AAOSGameState::AAOSGameState()
{
	StartTime = 0.0f;
	ElapsedTime = 0.0f;
}

void AAOSGameState::BeginPlay()
{
	Super::BeginPlay();

	
}

void AAOSGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsGameStarted)
	{
		ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
	}
}

void AAOSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, BlueTeamPlayers);
	DOREPLIFETIME(ThisClass, RedTeamPlayers);
	DOREPLIFETIME(ThisClass, LoadedItems);
	DOREPLIFETIME(ThisClass, ElapsedTime);
}

FItemInformation* AAOSGameState::GetItemInfoByID(int32 ItemID)
{
	for (FItemInformation& Item : LoadedItems)
	{
		if (Item.ItemID == ItemID)
		{
			return &Item;
		}
	}
	return nullptr;
}

void AAOSGameState::StartGame()
{
	StartTime = GetWorld()->GetTimeSeconds();
	bIsGameStarted = true;
}

void AAOSGameState::AddPlayerCharacter(AAOSCharacterBase* Character, ETeamSideBase TeamSide)
{
	switch (TeamSide)
	{
	case ETeamSideBase::Type:
		UE_LOG(LogTemp, Warning, TEXT("[AAOSGameState::AddCharacter] Failed to add character, team assignment is required."));
		break;
	case ETeamSideBase::Blue:
		UE_LOG(LogTemp, Log, TEXT("[AAOSGameState::AddCharacter] Successfully added character %s to Blue Team."), *Character->GetName());
		BlueTeamPlayers.Add(Character);
		break;
	case ETeamSideBase::Red:
		UE_LOG(LogTemp, Log, TEXT("[AAOSGameState::AddCharacter] Successfully added character %s to Red Team."), *Character->GetName());
		RedTeamPlayers.Add(Character);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[AAOSGameState::AddCharacter] Failed to add character, team assignment must be either Blue or Red team."));
		break;
	}
}

void AAOSGameState::RemovePlayerCharacter(AAOSCharacterBase* Character)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSGameState::RemovePlayerCharacter] Character is null."));
		return;
	}

	if ((INDEX_NONE != BlueTeamPlayers.Find(Character) || INDEX_NONE != RedTeamPlayers.Find(Character)))
	{
		BlueTeamPlayers.Remove(Character);
		RedTeamPlayers.Remove(Character);
	}
}

void AAOSGameState::MulticastBroadcastRespawnTime_Implementation(int32 PlayerIndex, float RemainingTime)
{
	if (OnRespawnTimerUpdated.IsBound())
	{
		UE_LOG(LogTemp, Log, TEXT("[AAOSGameState::ClientBroadcastRespawnTime] Broadcast OnRespawnTimerUpdated %d, %f"), PlayerIndex, RemainingTime);
		OnRespawnTimerUpdated.Broadcast(PlayerIndex, RemainingTime);
	}
}

void AAOSGameState::MulticastSendLoadedItems_Implementation(const TArray<FItemInformation>& Items)
{
	LoadedItems = Items;
}

void AAOSGameState::OnRep_PlayerCharactersReplicated()
{
	if (OnPlayerCharactersReplicated.IsBound())
	{
		OnPlayerCharactersReplicated.Broadcast();
	}
}

void AAOSGameState::OnRep_LoadedItemsReplicated()
{
	if (OnLoadedItemsUpdated.IsBound())
	{
		OnLoadedItemsUpdated.Broadcast(LoadedItems);
	}
}