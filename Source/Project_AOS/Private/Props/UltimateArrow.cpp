// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/UltimateArrow.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Structs/EnumCharacterType.h"
#include "GameFramework/ProjectileMovementComponent.h"

AUltimateArrow::AUltimateArrow()
{
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

	BoxCollision->SetBoxExtent(FVector(60.f, 60.f, 60.f));

	HitParticleSystem->Template = HitWorld;

	PrimaryActorTick.bCanEverTick = true;
}

void AUltimateArrow::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		HitParticleSystem->OnSystemFinished.AddDynamic(this, &ThisClass::OnParticleEnded);
	}
}

void AUltimateArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUltimateArrow::OnArrowHit_NetMulticast_Implementation(AActor* OtherActor, ECollisionChannel CollisionChannel)
{
	if (HasAuthority() == false)
	{
		DrawDebugSphere(GetWorld(), OtherActor->GetActorLocation(), 200.f, 12, FColor::Green, false, 1.0f);

		BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ArrowMesh->SetVisibility(false);

		switch (CollisionChannel)
		{
		case ECC_WorldStatic:
		case ECC_WorldDynamic:
			HitParticleSystem->Template = HitPlayer;
			break;

		case ECC_GameTraceChannel1: // AOSCharacter
			HitParticleSystem->Template = HitWorld;
			break;
		}

		HitParticleSystem->Activate(true);
		ArrowParticleSystem->Deactivate();
	}
}

void AUltimateArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (!HasAuthority() || OtherActor == Owner || OtherActor == this)
	{
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("[Server] %s Overlap Actor %s "), *GetName(), *OtherActor->GetName());

	ECollisionChannel CollisionChannel = OtherComp->GetCollisionObjectType();
	if (CollisionChannel == ECC_WorldStatic || CollisionChannel == ECC_WorldDynamic)
	{
		HandleWorldCollision(OtherActor);
	}
	else if (CollisionChannel == ECC_GameTraceChannel1)
	{
		HandleAOSCharacterCollision(OtherActor);

		// Attach to nearest enemy mesh
		AttachToNearestEnemyMesh(SweepResult.ImpactPoint);
	}
}

void AUltimateArrow::HandleWorldCollision(AActor* OtherActor)
{
	// Stop movement and disable collisions
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Attach to the other actor
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	AttachToActor(OtherActor, AttachmentRules);
}

void AUltimateArrow::HandleAOSCharacterCollision(AActor* OtherActor)
{
	ACharacterBase* HitCharacter = Cast<ACharacterBase>(OtherActor);
	if (!IsValid(HitCharacter))
	{
		return;
	}

	if (EnumHasAnyFlags(HitCharacter->ObjectType, EObjectType::Player) && HitCharacter->TeamSide != OwnerCharacter->TeamSide)
	{
		// Stop movement and disable collisions
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->ProjectileGravityScale = 0.0f;
		BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Notify hit via multicast
		OnArrowHit_NetMulticast(OtherActor, ECC_GameTraceChannel1);

		FCollisionQueryParams params;
		params.TraceTag = FName("Name_None");
		params.bTraceComplex = false;
		params.AddIgnoredActor(this);

		OutHits.Empty();

		bool bResult = GetWorld()->OverlapMultiByChannel(
			OutHits,
			GetActorLocation(),
			FQuat::Identity,
			ECC_GameTraceChannel3,
			FCollisionShape::MakeSphere(ArrowProperties.Radius),
			params
		);

		if (bResult)
		{
			for (const auto& OutHit : OutHits)
			{
				ACharacterBase* OverlapCharacter = Cast<ACharacterBase>(OutHit.GetActor());
				{
					UE_LOG(LogTemp, Warning, TEXT("[Server] %s Overlap Actor %s "), *GetName(), *OverlapCharacter->GetName());

					if (::IsValid(OverlapCharacter))
					{
						if (OverlapCharacter->TeamSide != OwnerCharacter->TeamSide)
						{
							ApplyDamage(OverlapCharacter, 0.0f);
						}
					}
				}
			}
		}
	}
}

void AUltimateArrow::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
{
	Super::OnParticleEnded(ParticleSystemComponent);

	DestroyActor_Server();
	Destroy();
}