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

void AArrow::OnArrowHit(const FHitResult& HitResult)
{
    if (HasAuthority() && HitResult.GetActor() && HitResult.GetActor() != GetOwner())
    {
        AActor* OtherActor = HitResult.GetActor();
        UPrimitiveComponent* OtherComp = HitResult.GetComponent();
        ECollisionChannel CollisionChannel = OtherComp->GetCollisionObjectType();

        // 캐릭터가 아닌 다른 물체에 맞은 경우 바로 충돌 처리
        ProjectileMovement->StopMovementImmediately();
        ProjectileMovement->ProjectileGravityScale = 0.0f;

        FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
        AttachToActor(OtherActor, AttachmentRules);

        BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        OnArrowHit_NetMulticast(CollisionChannel);
        ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
        switch (CollisionChannel)
        {
        case ECC_GameTraceChannel1: // AOSChampion
        case ECC_GameTraceChannel6: // Minion
            if (Character && OwnerCharacter->TeamSide != Character->TeamSide && OtherActor == TargetActor)
            {
                ApplyDamage(Character, 0.0f);
            }
            break;

        default:
            break;
        }
    }
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

        UParticleSystem* SelectedParticleSystem = nullptr;
        switch (CollisionChannel)
        {
        case ECC_GameTraceChannel1: // AOSChampion
        case ECC_GameTraceChannel6: // Minion
            SelectedParticleSystem = HitPlayer;
            break;

        default:
            SelectedParticleSystem = HitWorld;
            break;
        }

        if (SelectedParticleSystem)
        {
            HitParticleSystem->SetTemplate(SelectedParticleSystem);
            HitParticleSystem->Activate(true);
        }

        ArrowParticleSystem->Deactivate();
    }
}

void AArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (HasAuthority() && OtherActor != GetOwner())
    {
        OnArrowHit(SweepResult);
    }
}


void AArrow::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
{
	Super::OnParticleEnded(ParticleSystemComponent);

	DestroyActor_Server();
	Destroy();
}