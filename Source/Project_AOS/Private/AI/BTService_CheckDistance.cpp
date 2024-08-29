// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckDistance.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckDistance::UBTService_CheckDistance()
{
    NodeName = TEXT("Check Distance");
    Interval = 0.1f;
}

void UBTService_CheckDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;

    if (!AIController || !AIPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("AIController or AIPawn is null."));
        return;
    }

    AActor* TargetActor = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
    FVector StartChaseLocation = AIController->GetBlackboardComponent()->GetValueAsVector(ABaseAIController::LastSplineLocationKey);
    float MaxChaseDistance = AIController->GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::MaxChaseDistanceKey);

    if (TargetActor)
    {
        float DistanceToStartChase = FVector::Dist(AIPawn->GetActorLocation(), StartChaseLocation);

        if (DistanceToStartChase > MaxChaseDistance)
        {
            AIController->GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
            UE_LOG(LogTemp, Log, TEXT("Cleared TargetActorKey due to exceeding MaxChaseDistance. Current Distance: %f, Max Distance: %f"), DistanceToStartChase, MaxChaseDistance);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("TargetActor is null."));
    }
}

