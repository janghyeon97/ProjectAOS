// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/SplineActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

ASplineActor::ASplineActor()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ABILITY_E_MESH
	(TEXT("/Game/ProjectAOS/Blueprints/Box_5A41D919.Box_5A41D919"));
	if (ABILITY_E_MESH.Succeeded())
		StaticMesh = ABILITY_E_MESH.Object;

	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SetRootComponent(SplineComponent);

	//SplineCount = 0;

	

	bInClosedLoop = false;
}

void ASplineActor::UpdateSplineMesh()
{
	SplineComponent->SetClosedLoop(bInClosedLoop);

	float SplineLength = SplineComponent->GetSplineLength();
	float StaticMeshLength = 52.f;

	UE_LOG(LogTemp, Warning, TEXT("Spline Length: %f"), SplineLength);


	if (SplineMeshes.Num() > 0)
	{
		for (auto& SplineMesh : SplineMeshes)
		{
			SplineMesh->DestroyComponent();
		}

		SplineMeshes.Empty();
	}

	for (int SplineCount = 0; SplineCount < (FMath::RoundToZero(SplineLength / StaticMeshLength)) - 1; SplineCount++)
	{
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());

		SplineMeshComponent->SetStaticMesh(StaticMesh);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SplineMeshComponent->RegisterComponentWithWorld(GetWorld());
		SplineMeshComponent->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);

		StartLocations.Add(SplineComponent->GetLocationAtDistanceAlongSpline(SplineCount * StaticMeshLength, ESplineCoordinateSpace::Local));
		StartTangents.Add((SplineComponent->GetTangentAtDistanceAlongSpline(SplineCount * StaticMeshLength, ESplineCoordinateSpace::Local)).GetClampedToSize(1.f, 52.f));
		EndLocations.Add(SplineComponent->GetLocationAtDistanceAlongSpline((SplineCount + 1) * StaticMeshLength, ESplineCoordinateSpace::Local));
		EndTangents.Add((SplineComponent->GetTangentAtDistanceAlongSpline((SplineCount + 1) * StaticMeshLength, ESplineCoordinateSpace::Local)).GetClampedToSize(1.f, 52.f));

		SplineMeshComponent->SetStartAndEnd(StartLocations[SplineCount], StartTangents[SplineCount], EndLocations[SplineCount], EndTangents[SplineCount]);

		SplineMeshes.Add(SplineMeshComponent);
	}
}

void ASplineActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASplineActor::OnConstruction(const FTransform& Transform)
{
	
}

void ASplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

