// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Chase.generated.h"

class UPathFollowingComponent;
class UNavigationSystemV1;
class ACharacterBase;
class USplineComponent;
struct FAIRequestID;
struct FPathFollowingResult;

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UBTTask_Chase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Chase();

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    /** 그리드 점들을 생성합니다. */
    TArray<FVector> GenerateGridPoints(const FVector& Origin, const FVector& ForwardVector, const FVector& TargetLocation, float GridSize);
    int32 CalculateGridWidth(const FVector& Origin, const FVector& TargetLocation, float GridSize);

    /** 그리드의 각 점에 대해 장애물 정보를 수집합니다. */
    TArray<bool> PerformObstacleTrace(UWorld* World, ACharacterBase* Character, const TArray<FVector>& GridPoints);

    /** A* 알고리즘을 사용하여 최적 경로를 계산합니다. */
    TArray<FVector> AStarPathfinding(const TArray<FVector>& GridPoints, const TArray<bool>& ObstacleMap, const FVector& Start, const FVector& Goal, int32 GridWidth, int32 GridSize);
    bool IsDirectDiagonalMove(int32 IndexA, int32 IndexB, int32 GridWidth);
    float Heuristic(const FVector& A, const FVector& B);
    int32 FindClosestGridPointIndex(const FVector& Point, const TArray<FVector>& GridPoints);

    /** 경로를 뒤집습니다. */
    void ReversePath(TArray<FVector>&& OriginalPath);

    /** 현재 노드의 인덱스를 기반으로 인접한 이웃 노드를 찾습니다. */
    void GetNeighbors(int32 CurrentIndex, TArray<int32>& OutNeighbors, const TArray<FVector>& GridPoints, int32 GridWidth);

    /** 계산된 경로를 따라 AI 캐릭터를 이동시킵니다. */
    void MoveAlongPath(AAIController* AIController, TArray<FVector>&& Path);

    /** 경로를 시각화합니다. */
    void VisualizePath(const TArray<FVector>& Path, UWorld* World);

private:
	class UNavigationSystemV1* NavSystem = nullptr;

	FTimerHandle TimerHandle;
	int32 CurrentPointIndex = 0;

    TArray<FVector> CachedPath;
    bool bUsingCachedPath = false;
};
