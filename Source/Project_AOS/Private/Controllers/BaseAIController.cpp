// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName ABaseAIController::StartPositionKey(TEXT("StartPosition"));
const FName ABaseAIController::EndPositionKey(TEXT("EndPosition"));
const FName ABaseAIController::TargetActorKey(TEXT("TargetActor"));
const FName ABaseAIController::TargetSplineLocationKey(TEXT("TargetSplineLocation"));
const FName ABaseAIController::LastSplineLocationKey(TEXT("LastSplineLocation"));
const FName ABaseAIController::CurrentSplineDistanceKey(TEXT("CurrentSplineDistance"));
const FName ABaseAIController::MovementSpeedKey(TEXT("MovementSpeed"));
const FName ABaseAIController::MaxChaseDistanceKey(TEXT("MaxChaseDistance"));
const FName ABaseAIController::RangeKey(TEXT("Range"));
const FName ABaseAIController::IsAvoidingObstacleKey(TEXT("IsAvoidingObstacle"));
const FName ABaseAIController::IsAbilityReadyKey(TEXT("IsAbilityReady"));


ABaseAIController::ABaseAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard"));
	BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BrainComponent"));
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

}

void ABaseAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	EndAI();
}

void ABaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("[ABaseAIController::OnPossess] Possessed pawn: %s"), *InPawn->GetName());

	ACharacterBase* OwningCharacter = Cast<ACharacterBase>(InPawn);
	if (OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABaseAIController::OnPossess] Successfully possessed Character: %s"), *OwningCharacter->GetName());
		OwningCharacter->Controller = this;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ABaseAIController::OnPossess] Failed to possess ACharacterBase"));
	}
}