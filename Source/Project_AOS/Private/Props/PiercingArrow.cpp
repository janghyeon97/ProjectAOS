// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/PiercingArrow.h"
#include "Components/BoxComponent.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/SessionInfomation.h"

APiercingArrow::APiercingArrow()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ARROW_PARTICLE
	(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/DrawABead/FX/P_Sparrow_RMB.P_Sparrow_RMB"));
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

void APiercingArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void APiercingArrow::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		HitParticleSystem->OnSystemFinished.AddDynamic(this, &ThisClass::OnParticleEnded);
	}
}

void APiercingArrow::OnArrowHit_NetMulticast_Implementation(AActor* OtherActor, ECollisionChannel CollisionChannel)
{
	if (HasAuthority() == false)
	{
		switch (CollisionChannel)
		{
		case ECC_WorldStatic:
		case ECC_WorldDynamic:
			BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ArrowMesh->SetVisibility(false);
			ArrowParticleSystem->Deactivate();
			HitParticleSystem->Template = HitWorld;
			HitParticleSystem->Activate(true);
			break;

		case ECC_GameTraceChannel1: // AOSCharacter
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitPlayer, OtherActor->GetActorLocation(), GetActorRotation(), FVector(1), true, EPSCPoolMethod::None, true);
			break;

		default:
			HitParticleSystem->Template = HitWorld;
			HitParticleSystem->Activate(true);
			ArrowParticleSystem->Deactivate();
			break;
		}
	}
}

void APiercingArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (HasAuthority())
	{
		if (OtherActor != Owner)
		{
			//UE_LOG(LogTemp, Warning, TEXT("[Server] %s Overlap Actor %s "), *GetName(), *OtherActor->GetName());
			FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
			ECollisionChannel CollisionChannel = OtherComp->GetCollisionObjectType();

			switch (CollisionChannel)
			{
			case ECC_WorldStatic:
			case ECC_WorldDynamic:
				ProjectileMovement->StopMovementImmediately();
				ProjectileMovement->ProjectileGravityScale = 0.0f;

				BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				AttachToActor(OtherActor, AttachmentRules);
				OnArrowHit_NetMulticast(OtherActor, CollisionChannel);

				break;

			case ECC_GameTraceChannel1: // AOSCharacter
				ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
				if (::IsValid(Character))
				{
					if (OwnerCharacter->TeamSide != Character->TeamSide)
					{
						OnArrowHit_NetMulticast(OtherActor, CollisionChannel);
						ApplyDamage(OtherActor, ArrowProperties.Pierce_DamageReduction * Current_PierceCount);

						Current_PierceCount++;
					}
				}
				break;
			}
		}
	}
}

void APiercingArrow::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
{
	Super::OnParticleEnded(ParticleSystemComponent);

	DestroyActor_Server();
	Destroy();
}