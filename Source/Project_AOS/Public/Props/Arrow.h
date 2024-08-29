// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Props/ArrowBase.h"
#include "Arrow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AArrow : public AArrowBase
{
	GENERATED_BODY()
	
public:
	AArrow();

	virtual void Tick(float DeltaTime) override;

protected:
	void OnArrowHit(const FHitResult& HitResult);
	
	UFUNCTION(NetMulticast, Reliable)
	void OnArrowHit_NetMulticast(ECollisionChannel CollisionChannel);

protected:
	virtual void BeginPlay() override;

	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent);
};
