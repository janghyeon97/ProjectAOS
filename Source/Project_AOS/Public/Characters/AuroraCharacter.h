// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/TimelineComponent.h"
#include "AuroraCharacter.generated.h"


USTRUCT(BlueprintType)
struct FCachedParticleInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Lifetime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumParicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius;

	FCachedParticleInfo() : Lifetime(0), NumParicles(0), Scale(0), Rate(0), Radius(0) {}
	FCachedParticleInfo(int32 InLifetime, int32 InNumParicles, float InScale, float InRate, float InRadius) : Lifetime(InLifetime), NumParicles(InNumParicles), Scale(InScale), Rate(InRate), Radius(InRadius) {}
};

UCLASS()
class PROJECT_AOS_API AAuroraCharacter : public AAOSCharacterBase
{
	GENERATED_BODY()
	
public:
	AAuroraCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

protected:
	// Utility functions
	void Move(const FInputActionValue& InValue);
	void Look(const FInputActionValue& InValue);
	void SmoothMovement(float DeltaSeconds);
	bool ValidateAbilityUsage();
	int32 CalculateDirectionIndex();
	FVector CalculateTargetLocation(const FVector& MoveDirection, const float Rnage);

	// Ability activation functions
	virtual void Ability_Q() override;
	virtual void Ability_E() override;
	virtual void Ability_R() override;
	virtual void Ability_LMB() override;
	virtual void Ability_RMB() override;

	virtual void CancelAbility();
	virtual void Ability_E_Canceled();
	virtual void Ability_LMB_Canceled();
	virtual void Ability_RMB_Canceled();

	virtual void Ability_R_CheckHit() override;
	virtual void Ability_LMB_CheckHit() override;

	void StartComboAttack();
	void ContinueComboAttack();
	void HandleTumbling(float DeltaSeconds);
	void HandleDashing(float DeltaSeconds);

	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void OnPreDamageReceived(float FinalDamage) override;
	virtual void OnRep_CharacterStateChanged() override;
	virtual void OnRep_CrowdControlStateChanged() override;

	virtual void OnAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent) override;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Ability_RMB_Server(FVector TargetLocation);

private:
	void ExecuteSomethingSpecial();

	void Ability_LMB_AttackEnded();

	UFUNCTION(Server, Reliable)
	void SetMovementSpeed_Server(const float InMaxWalkSpeed, const float InMaxAcceleration);

	UFUNCTION(Server, Reliable)
	void Ability_E_Server(FVector TargetLocation);

	UFUNCTION(Server, Reliable)
	void Ability_R_Started_Server(const float BoostStrength);

	UFUNCTION(Server, Reliable)
	void SpawnFreezeSegments_Server(FTransform Transform, FCachedParticleInfo ParticleInfo, FDamageInformation DamageInformation);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnFreezeSegments_Multicast(FTransform Transform, FCachedParticleInfo ParticleInfo, FDamageInformation DamageInformation);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class", Meta = (AllowPrivateAccess))
	TObjectPtr<UClass> FreezeSegmentClass;

	TMap<int32, FCachedParticleInfo> CircularParticles;

	UPROPERTY(Replicated)
	FVector ReplicatedTargetLocation;

	UPROPERTY(Replicated)
	bool bSmoothMovement = false;

	bool bIsDashing;
	float Ability_E_Duration = 0.f;
	FVector Ability_E_TargetLocation = FVector();

	/* Ability LMB Combo Attack Count */
	uint8 Ability_LMB_CurrentComboCount : 3;
	uint8 Ability_LMB_MaxComboCount : 3;

	/* Ability RMB */
	bool bIsTumbling;

	float Ability_LastInterp;
	float Ability_ElapsedTime;

	FVector Ability_RMB_ControlPoint = FVector();
	FVector Ability_RMB_TargetLocation = FVector();

	float TumblingRotationSpeed = 360;

	FTimerHandle RecheckDataProviderHandle;
};
