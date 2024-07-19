// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/FreezeSegment.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AFreezeSegment::AFreezeSegment()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_SEGMENT(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Segment.P_Aurora_Freeze_Segment"));
	if (ABILITY_Q_SEGMENT.Succeeded()) FreezeSegment = ABILITY_Q_SEGMENT.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_ROOTED(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Rooted.P_Aurora_Freeze_Rooted"));
	if (ABILITY_Q_ROOTED.Succeeded()) FreezeRooted = ABILITY_Q_ROOTED.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_WHRILWIND(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Whrilwind.P_Aurora_Freeze_Whrilwind"));
	if (ABILITY_Q_WHRILWIND.Succeeded()) FreezeWhrilwind = ABILITY_Q_WHRILWIND.Object;

	Radius = 440.f;
	NumParicles = 28;
	Lifetime = 4;
	Rate = 0.01;
	Scale = 0.8f;
}

void AFreezeSegment::InitializeParticle(float InRadius, int32 InNumParicles, int32 InLifetime, float InRate, float InScale, FDamageInfomation InDamageInfomation)
{
	// 설정된 값들을 멤버 변수에 저장
	Radius = InRadius;
	NumParicles = InNumParicles;
	Lifetime = InLifetime;
	Rate = InRate;
	Scale = InScale;
	DamageInfomation = InDamageInfomation;
}


void AFreezeSegment::BeginPlay()
{
	Super::BeginPlay();

	LastActorLocation = GetActorLocation();
	LastActorForwardVector = GetActorForwardVector();
	LastActorUpVector = GetActorUpVector();

	Angle = 360.f / static_cast<float>(NumParicles);
	

	if (GetOwner() != nullptr)
	{
		OwnerCharacter = Cast<AAOSCharacterBase>(GetOwner());
	}

	if (HasAuthority())
	{
		for (int32 i = 0; i < NumParicles; ++i)
		{
			UBoxComponent* BoxComponent = NewObject<UBoxComponent>(this);
			BoxComponent->SetupAttachment(RootComponent);
			BoxComponent->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f)); // Box size
			BoxComponent->SetCollisionProfileName(TEXT("Projectile"));
			BoxComponent->bHiddenInGame = false;
			BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFreezeSegment::OnBeginOverlap);
			BoxComponent->RegisterComponent();

			CollisionBoxes.Add(BoxComponent);
		}
	}

	SpawnParticles();
}

void AFreezeSegment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFreezeSegment::SpawnParticles()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FreezeRooted, LastActorLocation, FRotator(0), FVector(1), true, EPSCPoolMethod::AutoRelease, true);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FreezeWhrilwind, LastActorLocation, FRotator(0), FVector(1), true, EPSCPoolMethod::AutoRelease, true);

	GetWorldTimerManager().SetTimer(
		ParticleTimer,
		[this]()
		{
			if (!FreezeSegment)
			{
				GetWorldTimerManager().ClearTimer(ParticleTimer);
				return;
			}

			const float AngleRad = FMath::DegreesToRadians(Iterator * Angle);

			FVector SpawnLocation = LastActorLocation + LastActorForwardVector.RotateAngleAxis(Iterator * Angle, LastActorUpVector) * Radius;
			FRotator SpawnRotation = LastActorForwardVector.Rotation();
			SpawnRotation.Yaw += Iterator * Angle + 90;

			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FreezeSegment, SpawnLocation, SpawnRotation, FVector(Scale), true, EPSCPoolMethod::AutoRelease, false);
			if (PSC)
			{
				PSC->SetFloatParameter(FName("AbilityDuration"), (NumParicles - Iterator - 1) * Rate + Lifetime);
				PSC->SetFloatParameter(FName("DurationRange"), (NumParicles - Iterator - 1) * Rate + Lifetime);
				PSC->Activate();
			}

			if (HasAuthority() && Iterator < CollisionBoxes.Num())
			{
				CollisionBoxes[Iterator]->SetWorldLocationAndRotation(SpawnLocation, SpawnRotation);
			}

			Iterator++;

			if (Iterator >= NumParicles)
			{
				GetWorldTimerManager().ClearTimer(ParticleTimer);
			}

		},
		Rate,
		true,
		0.0f
	);

	GetWorldTimerManager().SetTimer(
		ParticleEndTimer,
		[this]()
		{
			OnParticleEnded();
		},
		1.0f,
		false,
		NumParicles * Rate + Lifetime
	);
}

void AFreezeSegment::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (ProcessedActors.Contains(OtherActor))
	{
		return;
	}

	ECollisionChannel CollisionChannel = OtherComp->GetCollisionObjectType();
	if (CollisionChannel == ECC_WorldDynamic || CollisionChannel == ECC_WorldStatic)
	{
		return;
	}

	ProcessedActors.Add(OtherActor);
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Overlapped Actor %s"), *OtherActor->GetName()), true, true, FLinearColor::Red, 2.0);

	ApplyDamage(OtherActor);
}

void AFreezeSegment::ApplyDamage(AActor* OtherActor)
{
	ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
	if (::IsValid(Character))
	{
		OwnerCharacter->ApplyDamage_Server(Character, DamageInfomation, OwnerCharacter->GetController(), OwnerCharacter.Get());
	}
}


void AFreezeSegment::OnParticleEnded()
{
	Destroy();
}