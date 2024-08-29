// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MinionAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "Characters/MinionBase.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Components/SplineComponent.h"


AMinionAIController::AMinionAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardComponent> BLACKBOARD (TEXT("/Game/ProjectAOS/AI/Minion/BB_Minion.BB_Minion"));
	if (BLACKBOARD.Succeeded()) Blackboard = BLACKBOARD.Object;

	static ConstructorHelpers::FObjectFinder<UBehaviorTreeComponent> BRAINCOMPONENT (TEXT("/Game/ProjectAOS/AI/Minion/BT_Minion.BT_Minion"));
	if (BRAINCOMPONENT.Succeeded()) BrainComponent = BRAINCOMPONENT.Object;
}

void AMinionAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AMinionAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMinionAIController::BeginAI(APawn* InPawn)
{
    UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(Blackboard);
    if (::IsValid(BlackboardComponent))
    {
        if (UseBlackboard(BlackboardDataAsset, BlackboardComponent))
        {
            bool bRunSucceed = RunBehaviorTree(BehaviorTree);
            ensure(bRunSucceed == true);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AMinionAIController::BeginAI - Failed to use Blackboard."));
            return;
        }

        AMinionBase* Minion = Cast<AMinionBase>(InPawn);
        if (!Minion)
        {
            UE_LOG(LogTemp, Warning, TEXT("AMinionAIController::BeginAI - Failed to cast InPawn to AMinionBase."));
            return;
        }

        const UStatComponent* StatComponent = Minion->GetStatComponent();
        if (!StatComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("AMinionAIController::BeginAI - StatComponent is not valid for pawn: %s"), *InPawn->GetName());
            return;
        }

        const UAbilityStatComponent* AbilityStatComponent = Minion->GetAbilityStatComponent();
        if (!AbilityStatComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("AMinionAIController::BeginAI - AbilityStatComponent is not valid for pawn: %s"), *InPawn->GetName());
            return;
        }

        const float Range = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB).Range;
        const float Speed = StatComponent->GetMovementSpeed();
        const float MaxChaseDistance = Minion->MaxChaseDistance;

        BlackboardComponent->SetValueAsFloat(ABaseAIController::RangeKey, Range > 0 ? Range : 50.f);
        BlackboardComponent->SetValueAsFloat(ABaseAIController::MovementSpeedKey, Speed > 0 ? Speed : 300.f);
        BlackboardComponent->SetValueAsFloat(ABaseAIController::MaxChaseDistanceKey, MaxChaseDistance);

        USplineComponent* Spline = Minion->SplineActor->FindComponentByClass<USplineComponent>();
        if (Spline)
        {
            // Spline의 시작 위치를 계산하여 LastSplineLocationKey에 설정
            FVector SplineStartLocation = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
            BlackboardComponent->SetValueAsVector(ABaseAIController::TargetSplineLocationKey, SplineStartLocation);
            float SplineDistanceKey = EnumHasAnyFlags(Minion->TeamSide, ETeamSideBase::Blue) ? 0 : Spline->GetSplineLength();
            BlackboardComponent->SetValueAsFloat(ABaseAIController::CurrentSplineDistanceKey, SplineDistanceKey);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to find SplineComponent."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AMinionAIController::BeginAI - BlackboardComponent is not valid."));
    }
}



void AMinionAIController::EndAI()
{
	UBehaviorTreeComponent* BehaviroTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (::IsValid(BehaviroTreeComponent))
	{
		BehaviroTreeComponent->StopTree();
	}
}
