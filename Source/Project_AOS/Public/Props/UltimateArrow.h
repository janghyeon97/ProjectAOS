// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Props/ArrowBase.h"
#include "UltimateArrow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AUltimateArrow : public AArrowBase
{
	GENERATED_BODY()
	
public:
	AUltimateArrow();

	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void OnArrowHit_NetMulticast(AActor* OtherActor, ECollisionChannel CollisionChannel);

	virtual void BeginPlay() override;

	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent);

private:
	TArray<FOverlapResult> OutHits;
};
