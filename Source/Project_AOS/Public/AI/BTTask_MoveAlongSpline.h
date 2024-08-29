// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveAlongSpline.generated.h"

class AAIController;
class USplineComponent;
class AMinionBase;

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UBTTask_MoveAlongSpline : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_MoveAlongSpline();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
    FVector NextLocation;
    USplineComponent* Spline; // Spline을 클래스 변수로 선언

    void RecalculatePath(AAIController* AIController, AMinionBase* AICharacter);
    void UpdateNextLocation(AAIController* AIController); // 다음 목표 지점 업데이트 함수
    void VisualizePath(const FVector& StartLocation, const FVector& EndLocation);
};
