#include "AI/BTTask_Chase.h"
#include "AI/NavQueryFilter_AvoidCharacters.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/TimelineComponent.h"
#include "Containers/Set.h"
#include "Algo/Reverse.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"

UBTTask_Chase::UBTTask_Chase()
{
    NodeName = TEXT("Chase and Surround");
    bNotifyTick = true;
    bTickIntervals = 0.2f; // �ʿ信 ���� ����
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("[UBTTask_Chase::ExecuteTask] Failed to get AIController"));
        return EBTNodeResult::Failed;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[UBTTask_Chase::ExecuteTask] Failed to get AICharacter from AIController's Pawn"));
        return EBTNodeResult::Failed;
    }

    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    if (!TargetCharacter)
    {
        return EBTNodeResult::Failed;
    }

    const FVector StartLocation = AICharacter->GetActorLocation();
    const FVector ForwardVector = AICharacter->GetActorForwardVector();
    const FVector TargetLocation = TargetCharacter->GetActorLocation();

    const float GridSize = 100.0f;  // �׸����� �� ���� ũ��

    // ĳ���Ϳ� Ÿ�� ĳ���͸� �����ϴ� ���簢�� �׸��� ����
    const TArray<FVector>& GridPoints = GenerateGridPoints(StartLocation, ForwardVector, TargetLocation, GridSize);

    // ��ֹ� Ž���� ���� �� ����
    const TArray<bool>& ObstacleMap = PerformObstacleTrace(AICharacter->GetWorld(), AICharacter, GridPoints);

    // A* �˰������� ��� ã��
    int32 GridWidth = CalculateGridWidth(StartLocation, TargetLocation, GridSize);
    TArray<FVector>&& Path = AStarPathfinding(GridPoints, ObstacleMap, StartLocation, TargetLocation, GridWidth, GridSize);

    if (Path.Num() > 0)
    {
        // ��θ� ĳ���ϰ� �÷��� ����
        CachedPath = MoveTemp(Path);
        bUsingCachedPath = true;

        // ��� ���� �̵� ����
        //MoveAlongPath(AIController, MoveTemp(CachedPath));
        return EBTNodeResult::Succeeded;
    }
    else
    {
        return EBTNodeResult::Failed;
    }
}

void UBTTask_Chase::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    ACharacterBase* TargetCharacter = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));

    const FVector TargetLocation = TargetCharacter->GetActorLocation();
    const float Distance = FVector::Dist2D(AICharacter->GetActorLocation(), TargetLocation);
    const float Range = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);

    if (Distance <= Range)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::InProgress);
    }
    else if (FVector::Dist2D(CachedPath.Last(), TargetLocation) > Range * 0.1f)
    {
        bUsingCachedPath = false;
        ExecuteTask(OwnerComp, NodeMemory);
    }
}

int32 UBTTask_Chase::CalculateGridWidth(const FVector& Origin, const FVector& TargetLocation, float GridSize)
{
    FVector MinPoint = Origin.ComponentMin(TargetLocation);
    FVector MaxPoint = Origin.ComponentMax(TargetLocation);

    // X��� Y���� �Ÿ��� ����Ͽ� ������ �׸��� ũ�⸦ ���մϴ�.
    float GridWidth = (MaxPoint.X - MinPoint.X) / GridSize;
    float GridHeight = (MaxPoint.Y - MinPoint.Y) / GridSize;

    // ���簢�� �׸��带 ����� ���� ����/���� �� ū ���� �����մϴ�.
    return FMath::CeilToInt(FMath::Max(GridWidth, GridHeight)) + 1;
}


TArray<FVector> UBTTask_Chase::GenerateGridPoints(const FVector& Origin, const FVector& ForwardVector, const FVector& TargetLocation, float GridSize)
{
    TArray<FVector> GridPoints;

    // ForwardVector�� �������� ��ǥ�踦 ȸ����ŵ�ϴ�.
    const FRotator Rotation = ForwardVector.Rotation();

    // �׸����� ũ�⸦ ����մϴ�.
    int32 GridSizeCount = CalculateGridWidth(Origin, TargetLocation, GridSize);

    // Origin�� TargetLocation ������ �ּ�/�ִ� ��踦 ����մϴ�.
    FVector MinPoint = Origin.ComponentMin(TargetLocation);
    FVector MaxPoint = Origin.ComponentMax(TargetLocation);

    // Origin�� TargetLocation�� �׸����� �߾ӿ� ����������� �е��� �߰��մϴ�.
    FVector Padding = FVector(GridSize, GridSize, 0.0f);
    MinPoint -= Padding;
    MaxPoint += Padding;

    // �׸����� �߽��� ����մϴ�.
    FVector GridCenter = (MinPoint + MaxPoint) * 0.5f;

    int32 HalfSize = GridSizeCount / 2;

    // �׸����� ������ �����մϴ�.
    for (int32 x = -HalfSize; x <= HalfSize; x++)
    {
        for (int32 y = -HalfSize; y <= HalfSize; y++)
        {
            FVector LocalOffset = FVector(x * GridSize, y * GridSize, 0);
            FVector RotatedOffset = Rotation.RotateVector(LocalOffset);
            FVector GridPoint = GridCenter + RotatedOffset;

            GridPoints.Add(GridPoint);
        }
    }

    // �׸��� ����Ʈ �ε����� ȭ�鿡 ǥ���մϴ�.
    for (int32 i = 0; i < GridPoints.Num(); i++)
    {
        FString IndexString = FString::Printf(TEXT("%d"), i);
        DrawDebugString(GetWorld(), GridPoints[i], IndexString, nullptr, FColor::White, 2.0f, true);
    }

    return MoveTemp(GridPoints);
}




TArray<bool> UBTTask_Chase::PerformObstacleTrace(UWorld* World, ACharacterBase* Character, const TArray<FVector>& GridPoints)
{
    FCollisionQueryParams QueryParams(NAME_None, false, Character);
    TArray<bool> ObstacleMap;
    FHitResult HitResult;

    float SphereRadius = 25.0f;

    for (const FVector& Point : GridPoints)
    {
        bool bHit = World->SweepSingleByChannel(
            HitResult,
            Point + FVector(0, 0, 30.f),
            Point - FVector(0, 0, 30.f),
            FQuat::Identity,
            ECC_Visibility,
            FCollisionShape::MakeSphere(SphereRadius),
            QueryParams
        );
        ObstacleMap.Add(bHit);

        DrawDebugSphere(World, Point, SphereRadius, 12, bHit ? FColor::Red : FColor::Green, false, 2.0f);
    }

    return MoveTemp(ObstacleMap);
}


bool UBTTask_Chase::IsDirectDiagonalMove(int32 IndexA, int32 IndexB, int32 GridWidth)
{
    int32 RowA = IndexA / GridWidth;
    int32 RowB = IndexB / GridWidth;
    int32 ColA = IndexA % GridWidth;
    int32 ColB = IndexB % GridWidth;

    // �밢�� �̵��̶��, ��� ���� ���̰� ��� 1�̾�� �մϴ�.
    return FMath::Abs(RowA - RowB) == 1 && FMath::Abs(ColA - ColB) == 1;
}


// A* �˰��򿡼� ����� �޸���ƽ �Լ��Դϴ�.
float UBTTask_Chase::Heuristic(const FVector& A, const FVector& B)
{
    return FVector::Dist(A, B);
}

// A* �˰��� ����
TArray<FVector> UBTTask_Chase::AStarPathfinding(const TArray<FVector>& GridPoints, const TArray<bool>& ObstacleMap, const FVector& Start, const FVector& Goal, int32 GridWidth, int32 GridSize)
{
    struct Node
    {
        int32 Index;
        float G;
        float H;
        float F;
        TSharedPtr<Node> Parent;

        Node(int32 InIndex, float InG, float InH, TSharedPtr<Node> InParent = nullptr)
            : Index(InIndex), G(InG), H(InH), F(InG + InH), Parent(InParent) {}
    };

    TMap<int32, TSharedPtr<Node>> OpenListMap;
    TArray<TSharedPtr<Node>> OpenList;
    TSet<int32> ClosedSet;

    int32 StartIndex = FindClosestGridPointIndex(Start, GridPoints);
    int32 GoalIndex = FindClosestGridPointIndex(Goal, GridPoints);

    if (StartIndex == INDEX_NONE || GoalIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("[UBTTask_Chase::AStarPathfinding] Failed to find valid Start or Goal index. StartIndex: %d, GoalIndex: %d"), StartIndex, GoalIndex);
        return TArray<FVector>(); // �� �迭 ��ȯ
    }

    TSharedPtr<Node> StartNode = MakeShared<Node>(StartIndex, 0.0f, Heuristic(Start, Goal));
    OpenListMap.Add(StartIndex, StartNode);
    OpenList.Add(StartNode);

    while (OpenList.Num() > 0)
    {
        int32 BestNodeIndex = 0;
        for (int32 i = 1; i < OpenList.Num(); ++i)
        {
            if (OpenList[i]->F < OpenList[BestNodeIndex]->F)
            {
                BestNodeIndex = i;
            }
        }

        TSharedPtr<Node> CurrentNode = OpenList[BestNodeIndex];
        OpenList.RemoveAt(BestNodeIndex);

        if (CurrentNode->Index == GoalIndex)
        {
            TArray<FVector> Path;
            while (CurrentNode.IsValid())
            {
                Path.Add(GridPoints[CurrentNode->Index]);
                CurrentNode = CurrentNode->Parent;
            }

            UE_LOG(LogTemp, Log, TEXT("[UBTTask_Chase::AStarPathfinding] Path found successfully."));
            ReversePath(MoveTemp(Path));
            VisualizePath(Path, GetWorld());
            return Path;
        }

        ClosedSet.Add(CurrentNode->Index);
        UE_LOG(LogTemp, Log, TEXT("[UBTTask_Chase::AStarPathfinding] Added to ClosedSet: %d"), CurrentNode->Index);

        TArray<int32> Neighbors;
        GetNeighbors(CurrentNode->Index, Neighbors, GridPoints, GridWidth);

        for (int32 NeighborIndex : Neighbors)
        {
            if (NeighborIndex < 0 || NeighborIndex >= GridPoints.Num())
            {
                UE_LOG(LogTemp, Error, TEXT("[UBTTask_Chase::AStarPathfinding] Invalid NeighborIndex: %d, GridPoints.Num(): %d"), NeighborIndex, GridPoints.Num());
                continue;
            }

            if (ClosedSet.Contains(NeighborIndex))
            {
                UE_LOG(LogTemp, Log, TEXT("[UBTTask_Chase::AStarPathfinding] Neighbor already in ClosedSet: %d"), NeighborIndex);
                continue;
            }

            if (NeighborIndex != GoalIndex && ObstacleMap[NeighborIndex])
            {
                UE_LOG(LogTemp, Log, TEXT("[UBTTask_Chase::AStarPathfinding] Move blocked by obstacle at index: %d"), NeighborIndex);
                continue;
            }

            float TentativeG = CurrentNode->G + ((IsDirectDiagonalMove(CurrentNode->Index, NeighborIndex, GridWidth))
                ? FMath::Sqrt(2.0f) * GridSize
                : GridSize);

            TSharedPtr<Node>* NeighborNodePtr = OpenListMap.Find(NeighborIndex);
            if (!NeighborNodePtr)
            {
                TSharedPtr<Node> NeighborNode = MakeShared<Node>(NeighborIndex, TentativeG, Heuristic(GridPoints[NeighborIndex], Goal), CurrentNode);
                OpenListMap.Add(NeighborIndex, NeighborNode);
                OpenList.Add(NeighborNode);
                UE_LOG(LogTemp, Log, TEXT("[UBTTask_Chase::AStarPathfinding] Neighbor added to OpenList: %d"), NeighborIndex);
            }
            else if (TentativeG < (*NeighborNodePtr)->G)
            {
                TSharedPtr<Node>& NeighborNode = *NeighborNodePtr;
                NeighborNode->G = TentativeG;
                NeighborNode->F = NeighborNode->G + NeighborNode->H;
                NeighborNode->Parent = CurrentNode;
                UE_LOG(LogTemp, Log, TEXT("[UBTTask_Chase::AStarPathfinding] Neighbor in OpenList updated: %d"), NeighborIndex);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[UBTTask_Chase::AStarPathfinding] Failed to find a path from StartIndex: %d to GoalIndex: %d"), StartIndex, GoalIndex);
    return TArray<FVector>();
}




int32 UBTTask_Chase::FindClosestGridPointIndex(const FVector& Point, const TArray<FVector>& GridPoints)
{
    int32 ClosestIndex = INDEX_NONE;
    float MinDistanceSquared = FLT_MAX;

    for (int32 i = 0; i < GridPoints.Num(); i++)
    {
        float DistanceSquared = FVector::DistSquared(Point, GridPoints[i]);
        if (DistanceSquared < MinDistanceSquared)
        {
            MinDistanceSquared = DistanceSquared;
            ClosestIndex = i;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("FindClosestGridPointIndex: Point = %s, ClosestIndex = %d"), *Point.ToString(), ClosestIndex);
    return ClosestIndex;
}

void UBTTask_Chase::ReversePath(TArray<FVector>&& OriginalPath)
{
    Algo::Reverse(OriginalPath);
}

void UBTTask_Chase::GetNeighbors(int32 CurrentIndex, TArray<int32>& Neighbors, const TArray<FVector>& GridPoints, int32 GridWidth)
{
    Neighbors.Empty();

    // ���� ����� ��� �� ���
    int32 Row = CurrentIndex / GridWidth;
    int32 Col = CurrentIndex % GridWidth;

    // ������ �̵� ���� (8����)
    const TArray<FIntPoint> Directions = {
        FIntPoint(-1, -1), FIntPoint(-1, 0), FIntPoint(-1, 1), // ��� �»��, �߰� ���, ����
        FIntPoint(0, -1),  /* �߰� ���� */  FIntPoint(0, 1),   // ����, ����
        FIntPoint(1, -1),  FIntPoint(1, 0), FIntPoint(1, 1)    // �ϴ� ���ϴ�, �߰� �ϴ�, ���ϴ�
    };

    for (const FIntPoint& Direction : Directions)
    {
        int32 NeighborRow = Row + Direction.X;
        int32 NeighborCol = Col + Direction.Y;

        // �׸��� �������� Ȯ��
        if (NeighborRow >= 0 && NeighborRow < GridWidth && NeighborCol >= 0 && NeighborCol < GridWidth)
        {
            int32 NeighborIndex = NeighborRow * GridWidth + NeighborCol;
            Neighbors.Add(NeighborIndex);
        }
    }

    // �α� �޽����� �̿� ������ ���
    FString NeighborsLog = TEXT("[UBTTask_Chase::GetNeighbors] Neighbors for index ");
    NeighborsLog.AppendInt(CurrentIndex);
    NeighborsLog.Append(TEXT(": "));

    for (int32 NeighborIndex : Neighbors)
    {
        NeighborsLog.AppendInt(NeighborIndex);
        NeighborsLog.Append(TEXT(" "));
    }

    UE_LOG(LogTemp, Log, TEXT("%s"), *NeighborsLog);
}


void UBTTask_Chase::MoveAlongPath(AAIController* AIController, TArray<FVector>&& Path)
{
    if (Path.Num() == 0 || !AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveAlongPath: Invalid path or AIController"));
        return;
    }

    UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent();
    if (!PathFollowingComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveAlongPath: Invalid PathFollowingComponent"));
        return;
    }

    // ���� �̵��� �����ϰ� ���ο� �̵��� �����ϱ� ���� ��������Ʈ�� Ŭ�����մϴ�.
    PathFollowingComp->OnRequestFinished.Clear();

    int32 CurrentIndex = 0;

    auto MoveToNextPoint = [this, AIController, Path, &CurrentIndex, PathFollowingComp]()
        {
            if (CurrentIndex < Path.Num())
            {
                FAIMoveRequest MoveRequest;
                MoveRequest.SetGoalLocation(Path[CurrentIndex]);
                MoveRequest.SetAcceptanceRadius(10.0f);  // ���������� �󸶳� �������� �����ߴٰ� �Ǵ����� ����

                FNavPathSharedPtr PathPtr;
                FAIRequestID RequestID = PathFollowingComp->RequestMove(MoveRequest, PathPtr);

                if (RequestID.IsValid())
                {
                    UE_LOG(LogTemp, Log, TEXT("MoveAlongPath: Moving to point %d"), CurrentIndex);
                    CurrentIndex++;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("MoveAlongPath: Failed to start movement to point %d"), CurrentIndex);
                }
            }
        };

    // OnRequestFinished ��������Ʈ�� ����Ͽ� �̵��� �Ϸ�Ǿ��� �� ���� ����Ʈ�� �̵�
    PathFollowingComp->OnRequestFinished.AddLambda([this, MoveToNextPoint](FAIRequestID RequestID, const FPathFollowingResult& Result)
        {
            if (Result.Code == EPathFollowingResult::Success)
            {
                UE_LOG(LogTemp, Log, TEXT("MoveAlongPath: Successfully reached a point, moving to next."));
                MoveToNextPoint();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("MoveAlongPath: Movement failed or was blocked."));
            }
        });

    // ù ��° �̵� ����
    MoveToNextPoint();
}



void UBTTask_Chase::VisualizePath(const TArray<FVector>& Path, UWorld* World)
{
    if (Path.Num() < 2) return;

    const float LineThickness = 3.0f;
    const float PointSize = 10.0f;
    const float Duration = 2.0f; 

    for (int32 i = 0; i < Path.Num() - 1; i++)
    {
        // �� ����� ������ �����ϴ� ���� �׸��ϴ�.
        DrawDebugLine(World, Path[i], Path[i + 1], FColor::Blue, false, Duration, 0, LineThickness);

        // ����� �� ���� ���� ���� ǥ���մϴ�.
        DrawDebugSphere(World, Path[i], PointSize, 12, FColor::Green, false, Duration);
    }

    DrawDebugSphere(World, Path.Last(), PointSize, 12, FColor::Red, false, Duration);
}
