// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void BeginAI(APawn* InPawn) {};
	virtual void EndAI() {};

public:
	FTimerHandle PatrolTimerHandle = FTimerHandle();

	static const float PatrolRepeatInterval;
	static const float PatrolRadius;
	static const FName StartPatrolPositionKey;
	static const FName EndPatrolPositionKey;
	static const FName TargetActorKey;
	static const FName IsPlayerDetectedKey;
	static const FName IsGetCrowdControl;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseAIController", meta = (AllowPrivateAccess))
	TObjectPtr<class UBlackboardData> BlackboardDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseAIController", meta = (AllowPrivateAccess))
	TObjectPtr<class UBehaviorTree> BehaviorTree;
};
