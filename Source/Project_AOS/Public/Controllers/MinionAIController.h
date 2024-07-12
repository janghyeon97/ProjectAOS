// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/BaseAIController.h"
#include "MinionAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AMinionAIController : public ABaseAIController
{
	GENERATED_BODY()
	
public:
	AMinionAIController();

public:
	void BeginAI(APawn* InPawn);
	void EndAI();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
