// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestActor.generated.h"

UCLASS()
class PROJECT_AOS_API ATestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATestActor();

	void InitializeArrowActor(float Speed);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_ArrowSpeedChanged();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> HitWorld;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> HitPlayer;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class USceneComponent> DefaultSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class UStaticMeshComponent> ArrowMesh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess))
	TObjectPtr<class UBoxComponent> BoxCollision;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> ArrowParticleSystem;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> HitParticleSystem;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ProjectileMovement", meta = (AllowPrivateAccess))
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

private:
	UPROPERTY(ReplicatedUsing = OnRep_ArrowSpeedChanged)
	float ArrowSpeed = 0.f;
};
