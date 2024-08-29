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
    Spline = nullptr; // Spline 초기화
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

    // Spline을 초기화
    Spline = Minion->SplineActor->FindComponentByClass<USplineComponent>();
    if (!Spline)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_MoveAlongSpline::ExecuteTask - Spline component is null."));
        return EBTNodeResult::Failed;
    }

    // 초기 목표 위치 설정
    UpdateNextLocation(AIController);

    // AI 캐릭터를 다음 위치로 이동
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

    // 이전 위치와 비교하여 이동하지 않았을 경우 경로를 재탐색
    FVector LastLocation = AIController->GetBlackboardComponent()->GetValueAsVector(AMinionAIController::LastSplineLocationKey);
    float DistanceToLastLocation = FVector::Dist2D(CurrentLocation, LastLocation);

    if (DistanceToLastLocation < 10.f) // 10 units 이내의 차이만 있을 때 경로 재탐색
    {
        RecalculatePath(AIController, AICharacter);
        return;
    }

    //VisualizePath(CurrentLocation, NextLocation);

    // 목표 위치에 도달했는지 확인
    if (Distance < 100.f)
    {
        // 목표 위치에 도달하면 다음 목표 위치로 업데이트
        UpdateNextLocation(AIController);
        AIController->MoveToLocation(NextLocation, -1.0f, false, true, false, false, 0, true);
    }

    // 장애물 감지 및 경로 재탐색
    FVector ForwardVector = AICharacter->GetActorForwardVector();
    FVector EndLocation = CurrentLocation + ForwardVector * 100.f; // 100 units 앞을 검사
    FHitResult HitResult;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CurrentLocation,
        EndLocation,
        ECC_Visibility
    );

    if (bHit && HitResult.GetActor() && HitResult.GetActor() != AICharacter)
    {
        // 장애물이 감지되면 경로를 재탐색합니다.
        RecalculatePath(AIController, AICharacter);
        return;
    }

    // 현재 위치를 블랙보드에 업데이트
    AIController->GetBlackboardComponent()->SetValueAsVector(AMinionAIController::LastSplineLocationKey, CurrentLocation);
}
  

void UBTTask_MoveAlongSpline::RecalculatePath(AAIController* AIController, AMinionBase* AICharacter)
{
    // 경로를 재탐색하기 위해 기존 목표 지점을 조정
    UpdateNextLocation(AIController);

    // 새 목표 위치로 이동 시작
    AIController->MoveToLocation(NextLocation, -1.0f, false, true, false, false, 0, true);
}

void UBTTask_MoveAlongSpline::UpdateNextLocation(AAIController* AIController)
{
    if (!Spline)
    {
        return;
    }

    // 스플라인 상의 다음 목표 위치를 업데이트
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

    // 블랙보드 값을 업데이트하여 현재 진행 상태를 저장
    AIController->GetBlackboardComponent()->SetValueAsFloat(AMinionAIController::CurrentSplineDistanceKey, CurrentSplineDistance);
    AIController->GetBlackboardComponent()->SetValueAsVector(AMinionAIController::LastSplineLocationKey, NextLocation);
}

void UBTTask_MoveAlongSpline::VisualizePath(const FVector& StartLocation, const FVector& EndLocation)
{
    // StartLocation에서 EndLocation까지의 경로를 파란색 선으로 시각화합니다.
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, -1, 0, 2.0f);

    // 경로 상의 주요 지점에 구체를 그립니다.
    const int32 NumPoints = 10;
    for (int32 i = 1; i < NumPoints; i++)
    {
        FVector PointLocation = FMath::Lerp(StartLocation, EndLocation, i / static_cast<float>(NumPoints));
        DrawDebugSphere(GetWorld(), PointLocation, 10.0f, 12, FColor::Yellow, false, -1, 0, 1.0f);
    }
}