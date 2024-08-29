// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PlayerStateSave.h"

UPlayerStateSave::UPlayerStateSave()
{
	TeamSide = ETeamSideBase::Type;

	PlayerUniqueID = NAME_None;
	PlayerIndex = -1;
	SelectedChampionIndex = -1;
	SelectedChampionName = NAME_None;
}
