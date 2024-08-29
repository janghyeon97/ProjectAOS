// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckForEnemies.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UBTService_CheckForEnemies : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CheckForEnemies();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
