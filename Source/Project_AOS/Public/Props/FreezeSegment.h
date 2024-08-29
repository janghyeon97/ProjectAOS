// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/CustomCombatData.h"
#include "FreezeSegment.generated.h"

UCLASS()
class PROJECT_AOS_API AFreezeSegment : public AActor
{
	GENERATED_BODY()
	
public:	
	AFreezeSegment();

	void OnParticleEnded();
	void InitializeParticle(float InRadius, int32 InNumParicles, int32 InLifetime, float InRate, float InScale, FDamageInformation InDamageInfomation);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void SpawnParticles();
	virtual void ApplyDamage(AActor* OtherActor);

	// Overlap event functions
	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY()
	TWeakObjectPtr<class AAOSCharacterBase> OwnerCharacter;

	UPROPERTY()
	TArray<class UBoxComponent*> CollisionBoxes;

	UPROPERTY()
	TArray<class UParticleSystemComponent*> Particles;

	UPROPERTY()
	TSet<AActor*> ProcessedActors; // 중복 검사 방지

	FDamageInformation DamageInformation;

private:
	FTimerHandle ParticleTimer;
	FTimerHandle ParticleEndTimer;

	FVector LastActorLocation = FVector::ZeroVector;
	FVector LastActorForwardVector = FVector::ZeroVector;
	FVector LastActorUpVector = FVector::ZeroVector;

	int32 Iterator = 0;
	int32 Lifetime = 0;
	int32 NumParicles = 0;

	float Radius = 0.f;
	float Rate = 0.f;
	float Angle = 0.f;
	float Scale = 0.f;

	UParticleSystem* FreezeSegment = nullptr;
	UParticleSystem* FreezeRooted = nullptr;
	UParticleSystem* FreezeWhrilwind = nullptr;
};
