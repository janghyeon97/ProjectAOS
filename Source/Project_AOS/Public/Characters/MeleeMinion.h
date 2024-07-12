// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/MinionBase.h"
#include "MeleeMinion.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AMeleeMinion : public AMinionBase
{
	GENERATED_BODY()
	
public:
	AMeleeMinion();

public:
	virtual void BeginPlay() override;
};
