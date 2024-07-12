// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PlayerStateSave.h"

UPlayerStateSave::UPlayerStateSave()
{
	TeamSide = ETeamSideBase::Type;

	PlayerIndex = -1;
	SelectedChampionIndex = -1;
}
