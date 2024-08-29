// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "Structs/EnumMinionType.h"
#include "MinionBase.generated.h"

/**
 *
 */
UCLASS()
class PROJECT_AOS_API AMinionBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	AMinionBase();

protected:
	virtual void BeginPlay() override;
	virtual void SetWidget(class UUserWidgetBase* InUserWidgetBase) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void InitializeCharacterResources() override;

	// Character death and related functions
	UFUNCTION()
	virtual void OnCharacterDeath();
	UFUNCTION(NetMulticast, Reliable)
	virtual void OnCharacterDeath_Multicast();
	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void StartFadeOut();
	void EnableRagdoll();
	void ApplyDirectionalImpulse();

	virtual void Ability_LMB() override;
	virtual void Ability_LMB_CheckHit() override;
	void AttackEnded();

	// Experience distribution
	void FindNearbyPlayers(TArray<ACharacterBase*>& PlayerCharacters, ETeamSideBase InTeamSide, float Distance);
	void DistributeExperience(ACharacterBase* Eliminator, const TArray<ACharacterBase*>& NearbyEnemies);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastApplyImpulse(FVector Impulse);

	UFUNCTION()
	void OnRep_SkeletalMesh();

public:
	// Getter and Setter functions for Bounty
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	float GetExpBounty() const;
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	int32 GetGoldBounty() const;
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	void SetExpBounty(float NewExpBounty);
	UFUNCTION(BlueprintCallable, Category = "Bounty")
	void SetGoldBounty(int32 NewGoldBounty);

	// Direction calculation
	UFUNCTION(BlueprintCallable, Category = "Direction")
	int32 GetRelativeDirection(AActor* OtherActor) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MinionBase")
	TObjectPtr<class AMinionAIController> AIController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UCharacterWidgetComponent> WidgetComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_HPBar> HPBar;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	AActor* SplineActor;

	UPROPERTY()
	FName MinionType;

	UPROPERTY(ReplicatedUsing = OnRep_SkeletalMesh)
	USkeletalMesh* ReplicatedSkeletalMesh;

	UPROPERTY()
	int32 RelativeDirection = 0;

	// Experience sharing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExperienceShareRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, float> ShareFactor;

	// Bounty variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounty")
	float ExpBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounty")
	int32 GoldBounty;

	// Ragdoll and fade out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MinionBase")
	float RagdollBlendTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MinionBase")
	float ImpulseStrength = 1000.0f;

	UPROPERTY()
	float MaxChaseDistance = 2000.f;

	FTimerHandle DeathMontageTimerHandle;
	FTimerHandle FadeOutTimerHandle;

	float CurrentFadeDeath = 0.f;
	float FadeOutDuration = 1.0f;

	uint8 Ability_LMB_CurrentComboCount = 1;
	uint8 Ability_LMB_MaxComboCount = 4;
};
