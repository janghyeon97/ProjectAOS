// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineActor.generated.h"

UCLASS()
class PROJECT_AOS_API ASplineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASplineActor();

	void UpdateSplineMesh();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform);

public:
	UPROPERTY(VisibleAnywhere, Category = "Spline")
	class USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	class UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	bool bInClosedLoop;

	TArray<class USplineMeshComponent*> SplineMeshes;

	TArray<FVector> StartLocations;
	TArray<FVector> StartTangents;
	TArray<FVector> EndLocations;
	TArray<FVector> EndTangents;

	int SplineCount2;
};
