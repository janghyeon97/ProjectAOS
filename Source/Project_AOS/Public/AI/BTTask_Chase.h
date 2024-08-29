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
    /** �׸��� ������ �����մϴ�. */
    TArray<FVector> GenerateGridPoints(const FVector& Origin, const FVector& ForwardVector, const FVector& TargetLocation, float GridSize);
    int32 CalculateGridWidth(const FVector& Origin, const FVector& TargetLocation, float GridSize);

    /** �׸����� �� ���� ���� ��ֹ� ������ �����մϴ�. */
    TArray<bool> PerformObstacleTrace(UWorld* World, ACharacterBase* Character, const TArray<FVector>& GridPoints);

    /** A* �˰����� ����Ͽ� ���� ��θ� ����մϴ�. */
    TArray<FVector> AStarPathfinding(const TArray<FVector>& GridPoints, const TArray<bool>& ObstacleMap, const FVector& Start, const FVector& Goal, int32 GridWidth, int32 GridSize);
    bool IsDirectDiagonalMove(int32 IndexA, int32 IndexB, int32 GridWidth);
    float Heuristic(const FVector& A, const FVector& B);
    int32 FindClosestGridPointIndex(const FVector& Point, const TArray<FVector>& GridPoints);

    /** ��θ� �������ϴ�. */
    void ReversePath(TArray<FVector>&& OriginalPath);

    /** ���� ����� �ε����� ������� ������ �̿� ��带 ã���ϴ�. */
    void GetNeighbors(int32 CurrentIndex, TArray<int32>& OutNeighbors, const TArray<FVector>& GridPoints, int32 GridWidth);

    /** ���� ��θ� ���� AI ĳ���͸� �̵���ŵ�ϴ�. */
    void MoveAlongPath(AAIController* AIController, TArray<FVector>&& Path);

    /** ��θ� �ð�ȭ�մϴ�. */
    void VisualizePath(const TArray<FVector>& Path, UWorld* World);

private:
	class UNavigationSystemV1* NavSystem = nullptr;

	FTimerHandle TimerHandle;
	int32 CurrentPointIndex = 0;

    TArray<FVector> CachedPath;
    bool bUsingCachedPath = false;
};
