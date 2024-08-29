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
	virtual void OnPossess(APawn* InPawn) override;

public:
	virtual void BeginAI(APawn* InPawn) {};
	virtual void EndAI() {};

public:
	FTimerHandle PatrolTimerHandle = FTimerHandle();
	
	static const FName StartPositionKey;
	static const FName EndPositionKey;
	static const FName TargetActorKey;
	static const FName TargetSplineLocationKey;
	static const FName LastSplineLocationKey;
	static const FName CurrentSplineDistanceKey;
	static const FName MovementSpeedKey;
	static const FName MaxChaseDistanceKey;
	static const FName RangeKey;
	static const FName IsAvoidingObstacleKey;
	static const FName IsAbilityReadyKey;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseAIController", meta = (AllowPrivateAccess))
	TObjectPtr<class UBlackboardData> BlackboardDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseAIController", meta = (AllowPrivateAccess))
	TObjectPtr<class UBehaviorTree> BehaviorTree;
};
