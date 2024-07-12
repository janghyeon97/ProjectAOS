// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/TestActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ATestActor::ATestActor()
{
	DefaultSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	DefaultSceneComponent->SetupAttachment(GetRootComponent());

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(DefaultSceneComponent);
	BoxCollision->SetBoxExtent(FVector(20.f, 10.f, 10.f));
	BoxCollision->SetRelativeLocationAndRotation(FVector(135.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f));

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMeshComponent"));
	ArrowMesh->SetupAttachment(BoxCollision);
	ArrowMesh->SetRelativeLocationAndRotation(FVector(-50.f, 0.f, 0.f), FRotator(90.f, 0.f, 0.f));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetAutoActivate(false);
	ProjectileMovement->InitialSpeed = 0;
	ProjectileMovement->MaxSpeed = 0;

	HitParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("HitParticle"));
	HitParticleSystem->SetupAttachment(ArrowMesh);
	HitParticleSystem->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -20.f), FRotator(90.f, 0.f, 0.f));
	HitParticleSystem->SetAutoActivate(false);
	HitParticleSystem->bAutoDestroy = true;

	ArrowParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ArrowParticle"));
	ArrowParticleSystem->SetupAttachment(ArrowMesh);
	ArrowParticleSystem->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -55.f), FRotator(-90.f, 0.f, 0.f));
	ArrowParticleSystem->SetAutoActivate(true);
	ArrowParticleSystem->bAutoDestroy = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ARROW_MESH
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Meshes/Heroes/Sparrow/Abilities/SM_Sparrow_Arrow.SM_Sparrow_Arrow"));
	if (ARROW_MESH.Succeeded())
		ArrowMesh->SetStaticMesh(ARROW_MESH.Object);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ARROW_PARTICLE
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Ultimate/FX/P_Arrow_Ultimate.P_Arrow_Ultimate"));
	if (ARROW_PARTICLE.Succeeded())
		ArrowParticleSystem->Template = ARROW_PARTICLE.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HIT_WORLD_IMPACT
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Ultimate/FX/P_Sparrow_UltHitWorld.P_Sparrow_UltHitWorld"));
	if (HIT_WORLD_IMPACT.Succeeded())
		HitWorld = HIT_WORLD_IMPACT.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HIT_PLAYER_IMPACT
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Ultimate/FX/P_Sparrow_UltHit.P_Sparrow_UltHit"));
	if (HIT_PLAYER_IMPACT.Succeeded())
		HitPlayer = HIT_PLAYER_IMPACT.Object;

	HitParticleSystem->Template = HitWorld;

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	HitParticleSystem->OnSystemFinished.AddDynamic(this, &ThisClass::OnParticleEnded);

	PrimaryActorTick.bCanEverTick = false;
}

void ATestActor::InitializeArrowActor(float Speed)
{
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ArrowSpeed = Speed;
}

void ATestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ArrowSpeed);
}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ATestActor::BeginPlay - Arrow speed %f"), ProjectileMovement->InitialSpeed);

	ProjectileMovement->Activate();
}

void ATestActor::OnRep_ArrowSpeedChanged()
{
	ProjectileMovement->InitialSpeed = ArrowSpeed;
	ProjectileMovement->MaxSpeed = ArrowSpeed;
	UE_LOG(LogTemp, Warning, TEXT("ATestActor::OnRep_ArrowSpeedChanged - Arrow speed %f"), ProjectileMovement->InitialSpeed);
}

void ATestActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != GetOwner())
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->ProjectileGravityScale = 0.0f;

		FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
		AttachToActor(OtherActor, AttachmentRules);

		BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ArrowMesh->SetVisibility(false);

		ECollisionChannel HitActorCollision = OtherComp->GetCollisionObjectType();
		switch (HitActorCollision)
		{
		case ECC_GameTraceChannel1: // AOSChampion
			HitParticleSystem->Template = HitPlayer;
			break;
		case ECC_GameTraceChannel2: // Enemy
			HitParticleSystem->Template = HitPlayer;
			break;
		default:
			HitParticleSystem->Template = HitWorld;
			break;
		}

		/*switch (HitActorCollision)
		{
		case ECC_GameTraceChannel1:
			UE_LOG(LogTemp, Warning, TEXT("AArrowActor::OnBeginOverlap AOS Champion Hit!!!"));
			break;
		case ECC_GameTraceChannel2:
			UE_LOG(LogTemp, Warning, TEXT("AArrowActor::OnBeginOverlap Enemy Hit!!!"));
			break;
		case ECC_WorldStatic:
			UE_LOG(LogTemp, Warning, TEXT("AArrowActor::OnBeginOverlap ECC_WorldStatic Hit!!!"));
			break;
		case ECC_WorldDynamic:
			UE_LOG(LogTemp, Warning, TEXT("AArrowActor::OnBeginOverlap ECC_WorldDynamic Hit!!!"));
			break;
		case ECC_Pawn:
			UE_LOG(LogTemp, Warning, TEXT("AArrowActor::OnBeginOverlap ECC_Pawn Hit!!!"));
			break;
		case ECC_Visibility:
			UE_LOG(LogTemp, Warning, TEXT("AArrowActor::OnBeginOverlap ECC_Visibility Hit!!!"));
			break;
		}		*/

		HitParticleSystem->Activate(true);
		ArrowParticleSystem->Deactivate();
	}
}

void ATestActor::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
{
	if (this)
	{
		this->Destroy();
	}
}
