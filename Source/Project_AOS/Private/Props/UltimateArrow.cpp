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

	PrimaryActorTick.bCanEverTick = false;
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

	if (HasAuthority())
	{
		if (OtherActor != Owner && OtherActor != this)
		{
			UE_LOG(LogTemp, Error, TEXT("[Server] %s Overlap Actor %s "), *GetName(), *OtherActor->GetName());

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
				break;

			case ECC_GameTraceChannel1:
				ACharacterBase* HitCharacter = Cast<ACharacterBase>(OtherActor);
				if (::IsValid(HitCharacter))
				{
					if (EnumHasAnyFlags(HitCharacter->ObjectType, EObjectType::Player) && HitCharacter->TeamSide != OwnerCharacter->TeamSide)
					{
						ProjectileMovement->StopMovementImmediately();
						ProjectileMovement->ProjectileGravityScale = 0.0f;

						BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

						OnArrowHit_NetMulticast(OtherActor, CollisionChannel);

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
							FCollisionShape::MakeSphere(200.f),
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