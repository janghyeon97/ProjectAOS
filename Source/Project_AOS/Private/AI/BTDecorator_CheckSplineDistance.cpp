// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTDecorator_CheckSplineDistance.h"
#include "Controllers/BaseAIController.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckSplineDistance::UBTDecorator_CheckSplineDistance()
{
	NodeName = TEXT("Check Spline Distance");
}

bool UBTDecorator_CheckSplineDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTDecorator_CheckSplineDistance::CalculateRawConditionValue - AIController is null."));
		return false;
	}

	AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTDecorator_CheckSplineDistance::CalculateRawConditionValue - AICharacter is null."));
		return false;
	}

	if (AICharacter->SplineActor)
	{
		USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
		if (Spline)
		{
			// ���� ��ġ�� Spline���� ���� ����� ���� ������ �Ÿ� ���
			FVector CurrentLocation = AICharacter->GetActorLocation();
			float ClosestDistance = Spline->FindInputKeyClosestToWorldLocation(CurrentLocation);
			FVector ClosestPoint = Spline->GetLocationAtSplineInputKey(ClosestDistance, ESplineCoordinateSpace::World);
			float DistanceToSpline = FVector::Dist(CurrentLocation, ClosestPoint);

			// �Ÿ� ���� �˻�: Spline���� �������� �Ǵ� Spline�� ���� �̵����� ����
			return (DistanceToSpline <= 200.f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UBTDecorator_CheckSplineDistance::CalculateRawConditionValue - Spline component is null."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTDecorator_CheckSplineDistance::CalculateRawConditionValue - SplineActor is null."));
	}

	return false;
}
