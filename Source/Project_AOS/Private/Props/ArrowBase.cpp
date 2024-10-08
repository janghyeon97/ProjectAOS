// Fill out your copyright notice in the Description page of Project Settings.

#include "Props/ArrowBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"

AArrowBase::AArrowBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	DefaultSceneComponent->SetupAttachment(GetRootComponent());

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(DefaultSceneComponent);
	BoxCollision->SetBoxExtent(FVector(20.f, 10.f, 10.f));
	BoxCollision->SetRelativeLocationAndRotation(FVector(135.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f));
	BoxCollision->SetCollisionProfileName(FName("Projectile"));
	BoxCollision->BodyInstance.bUseCCD = true;

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMeshComponent"));
	ArrowMesh->SetupAttachment(BoxCollision);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetRelativeLocationAndRotation(FVector(-50.f, 0.f, 0.f), FRotator(90.f, 0.f, 0.f));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetAutoActivate(false);
	ProjectileMovement->InitialSpeed = 0;
	ProjectileMovement->MaxSpeed = 0;
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->bSweepCollision = true;
	ProjectileMovement->MaxSimulationTimeStep = 0.0166f;

	HitParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("HitParticle"));
	HitParticleSystem->SetupAttachment(ArrowMesh);
	HitParticleSystem->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -20.f), FRotator(90.f, 0.f, 0.f));
	HitParticleSystem->SetAutoActivate(false);
	HitParticleSystem->bAutoDestroy = true;
	HitParticleSystem->SetIsReplicated(true);

	ArrowParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ArrowParticle"));
	ArrowParticleSystem->SetupAttachment(ArrowMesh);
	ArrowParticleSystem->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -55.f), FRotator(-90.f, 0.f, 0.f));
	ArrowParticleSystem->SetAutoActivate(false);
	ArrowParticleSystem->bAutoDestroy = true;
	ArrowParticleSystem->SetIsReplicated(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ARROW_MESH
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Meshes/Heroes/Sparrow/Abilities/SM_Sparrow_Arrow.SM_Sparrow_Arrow"));
	if (ARROW_MESH.Succeeded())
		ArrowMesh->SetStaticMesh(ARROW_MESH.Object);

	bIsDestroyed = false;

	PrimaryActorTick.bCanEverTick = true;
}

void AArrowBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, ArrowProperties);
	DOREPLIFETIME(ThisClass, DamageInformation);
	DOREPLIFETIME(ThisClass, bIsDestroyed);
}

void AArrowBase::InitializeArrowActor(AActor* InTargetActor, FArrowProperties InArrowProperties, FDamageInformation InDamageInfomation)
{
	ProjectileMovement->InitialSpeed = InArrowProperties.Speed;
	ProjectileMovement->MaxSpeed = InArrowProperties.Speed;

	TargetActor = InTargetActor;
	ArrowProperties = InArrowProperties;
	DamageInformation = InDamageInfomation;
}

void AArrowBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && ::IsValid(Owner))
	{
		if (ArrowProperties.Range != 0)
		{
			FVector ArrowLocation = GetActorLocation();
			float Distance = static_cast<float>(FVector::Dist2D(OwnerLocation, ArrowLocation));
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Owner: %s, Range: %f, Distance: %f"), *Owner->GetName(), ArrowProperties.Range, Distance), true, true, FLinearColor::Green, 2.0f, "4");

			if (Distance >= ArrowProperties.Range)
			{
				Destroy(true);
			}
		}
	}

	if (HasAuthority() && GetVelocity().SizeSquared() > 0.0f)
	{
		FVector StartLocation = GetActorLocation();
		FVector EndLocation = StartLocation + (GetVelocity() * DeltaTime);

		FHitResult HitResult;

		// Line Trace (Raycast) 실행
		bool bHasHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			ECC_Visibility,  // 적절한 충돌 채널을 선택 (필요에 따라 다른 채널 사용 가능)
			FCollisionQueryParams(FName(TEXT("ArrowTrace")), true, this)
		);

		if (bHasHit)
		{
			OnArrowHit(HitResult);
		}
	}
}

void AArrowBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() != nullptr)
	{
		OwnerCharacter = Cast<AAOSCharacterBase>(GetOwner());
		OwnerLocation = Owner->GetActorLocation();
	}

	if (HasAuthority())
	{
		BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	}
	else
	{
		ArrowParticleSystem->Activate();
	}

	ProjectileMovement->Activate();
}

void AArrowBase::AttachToNearestEnemyMesh(const FVector& ImpactPoint)
{
	TArray<FHitResult> OutHits;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Owner);

	// Perform a sphere overlap to find nearby actors
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		ImpactPoint,
		ImpactPoint,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(200.f),
		QueryParams
	);

	if (bHit)
	{
		ACharacterBase* NearestCharacter = nullptr;
		float NearestDistance = FLT_MAX;

		// Find the nearest enemy character
		for (const FHitResult& Hit : OutHits)
		{
			ACharacterBase* OverlapCharacter = Cast<ACharacterBase>(Hit.GetActor());
			if (IsValid(OverlapCharacter) && OverlapCharacter->TeamSide != OwnerCharacter->TeamSide)
			{
				float Distance = FVector::Dist(ImpactPoint, OverlapCharacter->GetActorLocation());
				if (Distance < NearestDistance)
				{
					NearestDistance = Distance;
					NearestCharacter = OverlapCharacter;
				}
			}
		}

		if (NearestCharacter)
		{
			// Attach to the nearest character's mesh
			USkeletalMeshComponent* MeshComponent = NearestCharacter->FindComponentByClass<USkeletalMeshComponent>();
			if (MeshComponent)
			{
				FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
				AttachToComponent(MeshComponent, AttachmentRules);
				UE_LOG(LogTemp, Warning, TEXT("[Server] %s Attached to %s's Mesh"), *GetName(), *NearestCharacter->GetName());
			}
		}
	}
}

void AArrowBase::ApplyDamage(AActor* OtherActor, float DamageReduction)
{
	ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
	if (::IsValid(Character))
	{
		if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Physical))
		{
			DamageInformation.PhysicalDamage *= (1.0f - DamageReduction / 100.0f);
		}
		if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Magic))
		{
			DamageInformation.MagicDamage *= (1.0f - DamageReduction / 100.0f);
		}

		OwnerCharacter->ApplyDamage_Server(Character, DamageInformation, OwnerCharacter->GetController(), OwnerCharacter.Get());
	}
}

void AArrowBase::DestroyActor_Server_Implementation()
{
	if (::IsValid(this))
	{
		Destroy();
	}
}