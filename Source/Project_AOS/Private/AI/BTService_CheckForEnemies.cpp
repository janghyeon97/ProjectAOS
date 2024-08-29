// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckForEnemies.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CheckForEnemies::UBTService_CheckForEnemies()
{
	NodeName = TEXT("Check For Enemies");
	Interval = 0.1f;
}

void UBTService_CheckForEnemies::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return;
    }

    ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
    if (!AICharacter)
    {
        return;
    }

    if (OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey))
    {
        return;
    }

    FVector CenterPosition = AICharacter->GetActorLocation();
    const float DetectRadius = 500.f;

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.TraceTag = NAME_None;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(AICharacter);

    bool bResult = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        CenterPosition,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel4,
        FCollisionShape::MakeSphere(DetectRadius),
        QueryParams
    );

    if (!bResult)
    {
        AIController->GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
        return;
    }

    ACharacterBase* ClosestMinion = nullptr;
    ACharacterBase* ClosestPlayer = nullptr;
    float ClosestMinionDistance = FLT_MAX;
    float ClosestPlayerDistance = FLT_MAX;

    for (const auto& OverlapResult : OverlapResults)
    {
        ACharacterBase* Character = Cast<ACharacterBase>(OverlapResult.GetActor());
        if (!::IsValid(Character))
        {
            UE_LOG(LogTemp, Warning, TEXT("OverlapResult actor is invalid."));
            continue;
        }

        // 같은 팀의 캐릭터는 무시
        if (AICharacter->TeamSide == Character->TeamSide)
        {
            continue;
        }

        // 미니언일 경우
        if (EnumHasAnyFlags(Character->ObjectType, EObjectType::Minion))
        {
            float DistanceToMinion = FVector::Dist(CenterPosition, Character->GetActorLocation());
            if (DistanceToMinion < ClosestMinionDistance)
            {
                ClosestMinion = Character;
                ClosestMinionDistance = DistanceToMinion;
            }
        }
        // 플레이어일 경우
        else if (EnumHasAnyFlags(Character->ObjectType, EObjectType::Player))
        {
            float DistanceToPlayer = FVector::Dist(CenterPosition, Character->GetActorLocation());
            if (DistanceToPlayer < ClosestPlayerDistance)
            {
                ClosestPlayer = Character;
                ClosestPlayerDistance = DistanceToPlayer;
            }
        }
    }

    // 우선순위에 따라 타겟 설정
    if (ClosestMinion)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsObject(ABaseAIController::TargetActorKey, ClosestMinion);
    }
    else if (ClosestPlayer)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsObject(ABaseAIController::TargetActorKey, ClosestPlayer);
    }
    else
    {
        AIController->GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
    }

    // Debug sphere for visualizing the detection radius
    // DrawDebugSphere(GetWorld(), CenterPosition, DetectRadius, 16, FColor::Green, false, Interval);
}

