// UBTTask_MoveAlongSpline.cpp

#include "AI/BTTask_MoveAlongSpline.h"
#include "Controllers/MinionAIController.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

UBTTask_MoveAlongSpline::UBTTask_MoveAlongSpline()
{
    NodeName = TEXT("Move Along Spline");
    bNotifyTick = true;
    Spline = nullptr; // Spline �ʱ�ȭ
}

EBTNodeResult::Type UBTTask_MoveAlongSpline::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AMinionAIController* AIController = Cast<AMinionAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_MoveAlongSpline::ExecuteTask - AIController is null."));
        return EBTNodeResult::Failed;
    }

    AMinionBase* Minion = Cast<AMinionBase>(AIController->GetPawn());
    if (!Minion)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_MoveAlongSpline::ExecuteTask - Minion is null."));
        return EBTNodeResult::Failed;
    }

    if (!Minion->SplineActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_MoveAlongSpline::ExecuteTask - SplineActor is null."));
        return EBTNodeResult::Failed;
    }

    // Spline�� �ʱ�ȭ
    Spline = Minion->SplineActor->FindComponentByClass<USplineComponent>();
    if (!Spline)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_MoveAlongSpline::ExecuteTask - Spline component is null."));
        return EBTNodeResult::Failed;
    }

    // �ʱ� ��ǥ ��ġ ����
    UpdateNextLocation(AIController);

    // AI ĳ���͸� ���� ��ġ�� �̵�
    AIController->MoveToLocation(NextLocation, -1.0f, false, true, false, false, 0, true);

    return EBTNodeResult::InProgress;
}

void UBTTask_MoveAlongSpline::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector CurrentLocation = AICharacter->GetActorLocation();
    float Distance = FVector::Dist2D(CurrentLocation, NextLocation);

    // ���� ��ġ�� ���Ͽ� �̵����� �ʾ��� ��� ��θ� ��Ž��
    FVector LastLocation = AIController->GetBlackboardComponent()->GetValueAsVector(AMinionAIController::LastSplineLocationKey);
    float DistanceToLastLocation = FVector::Dist2D(CurrentLocation, LastLocation);

    if (DistanceToLastLocation < 10.f) // 10 units �̳��� ���̸� ���� �� ��� ��Ž��
    {
        RecalculatePath(AIController, AICharacter);
        return;
    }

    //VisualizePath(CurrentLocation, NextLocation);

    // ��ǥ ��ġ�� �����ߴ��� Ȯ��
    if (Distance < 100.f)
    {
        // ��ǥ ��ġ�� �����ϸ� ���� ��ǥ ��ġ�� ������Ʈ
        UpdateNextLocation(AIController);
        AIController->MoveToLocation(NextLocation, -1.0f, false, true, false, false, 0, true);
    }

    // ��ֹ� ���� �� ��� ��Ž��
    FVector ForwardVector = AICharacter->GetActorForwardVector();
    FVector EndLocation = CurrentLocation + ForwardVector * 100.f; // 100 units ���� �˻�
    FHitResult HitResult;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CurrentLocation,
        EndLocation,
        ECC_Visibility
    );

    if (bHit && HitResult.GetActor() && HitResult.GetActor() != AICharacter)
    {
        // ��ֹ��� �����Ǹ� ��θ� ��Ž���մϴ�.
        RecalculatePath(AIController, AICharacter);
        return;
    }

    // ���� ��ġ�� �����忡 ������Ʈ
    AIController->GetBlackboardComponent()->SetValueAsVector(AMinionAIController::LastSplineLocationKey, CurrentLocation);
}
  

void UBTTask_MoveAlongSpline::RecalculatePath(AAIController* AIController, AMinionBase* AICharacter)
{
    // ��θ� ��Ž���ϱ� ���� ���� ��ǥ ������ ����
    UpdateNextLocation(AIController);

    // �� ��ǥ ��ġ�� �̵� ����
    AIController->MoveToLocation(NextLocation, -1.0f, false, true, false, false, 0, true);
}

void UBTTask_MoveAlongSpline::UpdateNextLocation(AAIController* AIController)
{
    if (!Spline)
    {
        return;
    }

    // ���ö��� ���� ���� ��ǥ ��ġ�� ������Ʈ
    float CurrentSplineDistance = AIController->GetBlackboardComponent()->GetValueAsFloat(AMinionAIController::CurrentSplineDistanceKey);
    const float Speed = AIController->GetBlackboardComponent()->GetValueAsFloat(AMinionAIController::MovementSpeedKey);

    if (EnumHasAnyFlags(AIController->GetPawn<AMinionBase>()->TeamSide, ETeamSideBase::Blue))
    {
        CurrentSplineDistance += Speed * GetWorld()->DeltaTimeSeconds;
    }
    else
    {
        const float SplineLength = Spline->GetSplineLength();
        CurrentSplineDistance -= Speed * GetWorld()->DeltaTimeSeconds;
        CurrentSplineDistance = FMath::Clamp(CurrentSplineDistance, 0.0f, SplineLength);
    }

    NextLocation = Spline->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);

    // ������ ���� ������Ʈ�Ͽ� ���� ���� ���¸� ����
    AIController->GetBlackboardComponent()->SetValueAsFloat(AMinionAIController::CurrentSplineDistanceKey, CurrentSplineDistance);
    AIController->GetBlackboardComponent()->SetValueAsVector(AMinionAIController::LastSplineLocationKey, NextLocation);
}

void UBTTask_MoveAlongSpline::VisualizePath(const FVector& StartLocation, const FVector& EndLocation)
{
    // StartLocation���� EndLocation������ ��θ� �Ķ��� ������ �ð�ȭ�մϴ�.
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, -1, 0, 2.0f);

    // ��� ���� �ֿ� ������ ��ü�� �׸��ϴ�.
    const int32 NumPoints = 10;
    for (int32 i = 1; i < NumPoints; i++)
    {
        FVector PointLocation = FMath::Lerp(StartLocation, EndLocation, i / static_cast<float>(NumPoints));
        DrawDebugSphere(GetWorld(), PointLocation, 10.0f, 12, FColor::Yellow, false, -1, 0, 1.0f);
    }
}