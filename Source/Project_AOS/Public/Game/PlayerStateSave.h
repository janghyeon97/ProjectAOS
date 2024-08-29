// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Game/AOSPlayerState.h"
#include "PlayerStateSave.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UPlayerStateSave : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPlayerStateSave();

public:
	UPROPERTY()
	FName PlayerUniqueID;

	UPROPERTY()
	ETeamSideBase TeamSide;

	UPROPERTY()
	int32 PlayerIndex;

	UPROPERTY()
	int32 SelectedChampionIndex;

	UPROPERTY()
	FName SelectedChampionName;
};
