// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_ReturnToSpline.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

UBTTask_ReturnToSpline::UBTTask_ReturnToSpline()
{
    NodeName = TEXT("Return To Spline");
    bNotifyTick = true;
    bIsReturning = false;
}

EBTNodeResult::Type UBTTask_ReturnToSpline::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_ReturnToSpline::ExecuteTask - AIController is null."));
        return EBTNodeResult::Failed;
    }

    AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_ReturnToSpline::ExecuteTask - AICharacter is null."));
        return EBTNodeResult::Failed;
    }

    if (AICharacter->SplineActor)
    {
        USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
        if (Spline)
        {
            MoveToClosestSplinePoint(OwnerComp);
            bIsReturning = true;
            return EBTNodeResult::InProgress;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UBTTask_ReturnToSpline::ExecuteTask - Spline component is null."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_ReturnToSpline::ExecuteTask - SplineActor is null."));
    }

    return EBTNodeResult::Failed;
}

void UBTTask_ReturnToSpline::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController || !AIController->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_ReturnToSpline::TickTask - AIController or Pawn is null, finishing task as failed."));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_ReturnToSpline::TickTask - AICharacter is null, finishing task as failed."));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector CurrentLocation = AICharacter->GetActorLocation();
    FVector TargetSplineLocation = AIController->GetBlackboardComponent()->GetValueAsVector(ABaseAIController::TargetSplineLocationKey);
    float MaxChaseDistance = AIController->GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::MaxChaseDistanceKey);
    float DetectRadius = 500.f;

    float DistanceToSpline = FVector::Dist(CurrentLocation, TargetSplineLocation);

    // 탐지 거리와 Spline까지의 거리의 합이 최대 추적 거리를 초과하지 않으면 적 탐지
    if (DistanceToSpline + DetectRadius <= MaxChaseDistance)
    {
        // 주변 적 탐지
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams CollisionQueryParams(NAME_None, false, AICharacter);

        bool bResult = GetWorld()->OverlapMultiByChannel(
            OverlapResults,
            CurrentLocation,
            FQuat::Identity,
            ECollisionChannel::ECC_GameTraceChannel4,
            FCollisionShape::MakeSphere(DetectRadius),
            CollisionQueryParams
        );

        if (bResult)
        {
            for (const auto& OverlapResult : OverlapResults)
            {
                ACharacterBase* Character = Cast<ACharacterBase>(OverlapResult.GetActor());
                if (::IsValid(Character) && AICharacter->TeamSide != Character->TeamSide)
                {
                    UE_LOG(LogTemp, Log, TEXT("UBTTask_ReturnToSpline::TickTask - Enemy detected, switching to attack mode."));
                    AIController->GetBlackboardComponent()->SetValueAsObject(ABaseAIController::TargetActorKey, Character);
                    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                    return;
                }
            }
        }
    }

    // 목표 Spline 포인트에 도달했는지 확인
    if (DistanceToSpline <= 200.f)
    {
        UE_LOG(LogTemp, Log, TEXT("UBTTask_ReturnToSpline::TickTask - Reached the target spline point, finishing task."));

        // 현재 위치를 기반으로 Spline 상의 거리를 계산하여 CurrentSplineDistanceKey를 업데이트
        USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
        if (Spline)
        {
            // 현재 위치와 Spline 상의 가장 가까운 점 사이의 거리를 계산
            float ClosestInputKey = Spline->FindInputKeyClosestToWorldLocation(AICharacter->GetActorLocation());
            float ClosestDistanceOnSpline = Spline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);

            // CurrentSplineDistanceKey를 업데이트
            AIController->GetBlackboardComponent()->SetValueAsFloat(ABaseAIController::CurrentSplineDistanceKey, ClosestDistanceOnSpline);

            // 복귀한 위치를 Spline 상의 LastSplineLocationKey로 업데이트
            AIController->GetBlackboardComponent()->SetValueAsVector(ABaseAIController::LastSplineLocationKey, AICharacter->GetActorLocation());
        }

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_ReturnToSpline::MoveToClosestSplinePoint(UBehaviorTreeComponent& OwnerComp)
{
    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());

    if (AICharacter && AICharacter->SplineActor)
    {
        USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
        if (Spline)
        {
            FVector CurrentLocation = AICharacter->GetActorLocation();
            float ClosestDistance = Spline->FindInputKeyClosestToWorldLocation(CurrentLocation);
            FVector ClosestPoint = Spline->GetLocationAtSplineInputKey(ClosestDistance, ESplineCoordinateSpace::World);

            // 목표 Spline 포인트를 블랙보드에 저장
            AIController->GetBlackboardComponent()->SetValueAsVector(ABaseAIController::TargetSplineLocationKey, ClosestPoint);

            UAIBlueprintHelperLibrary::SimpleMoveToLocation(AIController, ClosestPoint);
            UE_LOG(LogTemp, Log, TEXT("UBTTask_ReturnToSpline::MoveToClosestSplinePoint - Moving to closest spline point at location: %s"), *ClosestPoint.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UBTTask_ReturnToSpline::MoveToClosestSplinePoint - Spline component or AICharacter is null."));
    }
}
