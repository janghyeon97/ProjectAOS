// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckSplineDistance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UBTDecorator_CheckSplineDistance : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_CheckSplineDistance();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};