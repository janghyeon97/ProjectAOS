// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AOSCharacterBase.h"
#include "SparrowCharacter.generated.h"

/**
 * 스패로우 캐릭터 클래스입니다. 기본 AOS 캐릭터 기능에 더하여 스패로우 특유의 능력들을 구현합니다.
 */
UCLASS()
class PROJECT_AOS_API ASparrowCharacter : public AAOSCharacterBase
{
	GENERATED_BODY()

public:
	ASparrowCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Initialization functions
	void InitializeAbilityMontages();
	void InitializeAbilityParticles();

protected:
	// Utility functions
	void Move(const FInputActionValue& InValue);
	void Look(const FInputActionValue& InValue);

	// Ability functions
	virtual void Ability_Q() override;
	virtual void Ability_E() override;
	virtual void Ability_R() override;
	virtual void Ability_LMB() override;
	virtual void Ability_RMB() override;
	virtual void Ability_Q_CheckHit() override;
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void OnRep_CharacterStateChanged() override;

	void ExecuteAbilityR(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation);
	void ExecuteAbilityLMB(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation);
	UAnimMontage* GetMontageBasedOnAttackSpeed(float AttackSpeed);

	void ChangeCameraLength(float TargetLength);
	void Ability_Q_Fire();
	void Ability_RMB_Canceled();
	void Ability_RMB_Fire();

	virtual void ServerNotifyAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent) override;

private:
	bool ValidateAbilityUsage();

	UFUNCTION(BlueprintCallable, Meta = (AllowPrivateAccess))
	FVector GetImpactPoint(float TraceRange = 10000.f);

	UFUNCTION(Server, Reliable)
	void SpawnArrow_Server(UClass* SpawnArrowClass, FTransform SpawnTransform, FArrowProperties InArrowProperties, FDamageInfomation InDamageInfomation);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_LMB_FastMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_LMB_SlowMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|Q", Meta = (AllowPrivateAccess))
	float Ability_Q_Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|R", Meta = (AllowPrivateAccess))
	float Ability_R_ArrowSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|R", Meta = (AllowPrivateAccess))
	float Ability_R_SideDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|R", Meta = (AllowPrivateAccess))
	float Ability_R_Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|R", Meta = (AllowPrivateAccess))
	float Ability_R_Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|R", Meta = (AllowPrivateAccess))
	float Ability_R_Angle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|LMB", Meta = (AllowPrivateAccess))
	float Ability_LMB_ArrowSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|LMB", Meta = (AllowPrivateAccess))
	float Ability_LMB_Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|RMB", Meta = (AllowPrivateAccess))
	float Ability_RMB_ArrowSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparrow|AbilityStat|RMB", Meta = (AllowPrivateAccess))
	float Ability_RMB_Range;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> BowParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> Ability_Q_RainOfArrows;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> ArrowParticle_LMB;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystem> ArrowParticle_RMB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Arrow", Meta = (AllowPrivateAccess))
	TObjectPtr<UClass> BasicArrowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Arrow", Meta = (AllowPrivateAccess))
	TObjectPtr<UClass> PiercingArrowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Arrow", Meta = (AllowPrivateAccess))
	TObjectPtr<UClass> UltimateArrowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Arrow", Meta = (AllowPrivateAccess))
	TObjectPtr<UClass> TargetDecalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Arrow", Meta = (AllowPrivateAccess))
	TObjectPtr<AActor> TargetDecalActor;

	FVector CrosshairLocation = FVector::ZeroVector;
	FVector Ability_LMB_ImpactPoint = FVector::ZeroVector;
	bool bTraceImpactPoint = false;
};
