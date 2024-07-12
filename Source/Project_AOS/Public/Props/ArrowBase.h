// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/DamageInfomationStruct.h"
#include "ArrowBase.generated.h"

USTRUCT(BlueprintType)
struct FArrowProperties
{
	GENERATED_BODY()

public:
	FArrowProperties()
	{
		Speed = 0.f;
		Range = 0.f;
		PierceCount = 0;
		Pierce_DamageReduction = 0.f;
	};

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "20000.0", uIMin = "0.0", uIMax = "20000.0"))
	float Speed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float Range = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0", ClampMax = "100", uIMin = "0", uIMax = "100"))
	int PierceCount;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Arrow|Properties", meta = (ClampMin = "0.0", ClampMax = "1.0", uIMin = "0.0", uIMax = "1.0"))
	float Pierce_DamageReduction;
};

UCLASS()
class PROJECT_AOS_API AArrowBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AArrowBase();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void InitializeArrowActor(FArrowProperties InArrowProperties, FDamageInfomation InDamageInfomation);

protected:
	virtual void ApplyDamage(AActor* OtherActor, float DamageReduction);

	UFUNCTION(Server, Reliable)
	virtual void DestroyActor_Server();

	UFUNCTION()
	virtual void OnRep_ArrowDestroyed() {};

	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {};

	UFUNCTION()
	virtual void OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent) {};

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

protected:
	UPROPERTY()
	TWeakObjectPtr<class AAOSCharacterBase> OwnerCharacter;

	UPROPERTY(Replicated)
	FArrowProperties ArrowProperties;

	UPROPERTY(Replicated)
	FDamageInfomation DamageInfomation;

	UPROPERTY(ReplicatedUsing = OnRep_ArrowDestroyed)
	bool bIsDestroyed;

	FVector OwnerLocation = FVector::ZeroVector;

	int Current_PierceCount = 0;
};
