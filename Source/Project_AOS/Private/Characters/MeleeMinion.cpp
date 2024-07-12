// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MeleeMinion.h"
#include "Structs/MinionData.h"

AMeleeMinion::AMeleeMinion()
{
	MinionType = EMinionType::Melee;
}

void AMeleeMinion::BeginPlay()
{
	Super::BeginPlay();
}
