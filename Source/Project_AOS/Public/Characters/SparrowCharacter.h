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

	virtual void CancelAbility();
	virtual void Ability_Q_Canceled() override;
	virtual void Ability_RMB_Canceled() override;

	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void OnRep_CharacterStateChanged() override;
	virtual void OnRep_CrowdControlStateChanged() override;

	void ProcessImpactWithNonStaticActor(const FImpactResult& ImpactResult);
	void FindAndSetClosestBone(const FImpactResult& ImpactResult, USkeletalMeshComponent* SkeletalMeshComponent);
	void ExecuteAbilityR(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation);
	void ExecuteAbilityLMB(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation);
	UAnimMontage* GetMontageBasedOnAttackSpeed(float AttackSpeed);

	void ChangeCameraLength(float TargetLength);
	
	void Ability_Q_Fire();
	void Ability_RMB_Fire();

	virtual void OnAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent) override;

private:
	void ExecuteSomethingSpecial();
	bool ValidateAbilityUsage();

	UFUNCTION(Server, Reliable)
	void SpawnArrow_Server(UClass* SpawnArrowClass, AActor* TargetActor, FTransform SpawnTransform, FArrowProperties InArrowProperties, FDamageInformation InDamageInfomation);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Particles", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> BowParticleSystem;

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

private:
	float Ability_Q_Range = 0.f;
	FVector Ability_Q_DecalLocation = FVector::ZeroVector;


	FVector Ability_LMB_ImpactPoint = FVector::ZeroVector;
	bool bTraceImpactPoint = false;
};
