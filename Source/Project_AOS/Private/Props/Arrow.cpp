// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Arrow.h"
#include "Components/BoxComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/SessionInfomation.h"

AArrow::AArrow()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ARROW_PARTICLE
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_PrimaryAttack.P_Sparrow_PrimaryAttack"));
	if (ARROW_PARTICLE.Succeeded())
		ArrowParticleSystem->Template = ARROW_PARTICLE.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HIT_WORLD_IMPACT
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_Primary_Ballistic_HitWorld.P_Sparrow_Primary_Ballistic_HitWorld"));
	if (HIT_WORLD_IMPACT.Succeeded())
		HitWorld = HIT_WORLD_IMPACT.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HIT_PLAYER_IMPACT
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_Primary_Ballistic_HitPlayer.P_Sparrow_Primary_Ballistic_HitPlayer"));
	if (HIT_PLAYER_IMPACT.Succeeded())
		HitPlayer = HIT_PLAYER_IMPACT.Object;

	HitParticleSystem->Template = HitWorld;

	PrimaryActorTick.bCanEverTick = true;
}

void AArrow::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority() == false)
	{
		HitParticleSystem->OnSystemFinished.AddDynamic(this, &ThisClass::OnParticleEnded);
	}
}

void AArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AArrow::OnArrowHit_NetMulticast_Implementation(ECollisionChannel CollisionChannel)
{
	if (HasAuthority() == false)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->ProjectileGravityScale = 0.0f;

		BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ArrowMesh->SetVisibility(false);

		switch (CollisionChannel)
		{
		case ECC_GameTraceChannel1: // AOSChampion
			HitParticleSystem->Template = HitPlayer;
			break;

		default:
			HitParticleSystem->Template = HitWorld;
			break;
		}

		HitParticleSystem->Activate(true);
		ArrowParticleSystem->Deactivate();
	}
}

void AArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (HasAuthority())
	{
		if (OtherActor != GetOwner())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Server] %s Overlap Actor %s "), *GetName(), *OtherActor->GetName());

			ProjectileMovement->StopMovementImmediately();
			ProjectileMovement->ProjectileGravityScale = 0.0f;

			FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
			AttachToActor(OtherActor, AttachmentRules);

			BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			ECollisionChannel CollisionChannel = OtherComp->GetCollisionObjectType();
			OnArrowHit_NetMulticast(CollisionChannel);

			switch (CollisionChannel)
			{
			case ECC_GameTraceChannel1: // AOSChampion
			case ECC_GameTraceChannel6: // Minion
				ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
				if (::IsValid(Character) == false)
				{
					return;
				}

				if (OwnerCharacter->TeamSide != Character->TeamSide)
				{
					AttachToNearestEnemyMesh(SweepResult.ImpactPoint);
					ApplyDamage(Character, 0.0f);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[AArrow::OnBeginOverlap] %s, Overlap Actor %s is same team."), *GetName(), *OtherActor->GetName());
				}

				break;
			}
		}
	}
}

void AArrow::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
{
	Super::OnParticleEnded(ParticleSystemComponent);

	DestroyActor_Server();
	Destroy();
}