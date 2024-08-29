// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsAbilityReady.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UBTDecorator_IsAbilityReady : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_IsAbilityReady();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
