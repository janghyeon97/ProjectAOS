// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/BaseAIController.h"
#include "NPCAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API ANPCAIController : public ABaseAIController
{
	GENERATED_BODY()
	
public:
	ANPCAIController();

public:
	virtual void BeginAI(APawn* InPawn) override;
	virtual void EndAI() override;

private:
	void OnPatrolTimerElapsed();
};
