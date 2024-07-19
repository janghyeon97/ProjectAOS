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

	// Character death and related functions
	UFUNCTION()
	virtual void OnCharacterDeath();
	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void StartFadeOut();
	void EnableRagdoll();
	void ApplyDirectionalImpulse();

	// Experience distribution
	void FindNearbyPlayers(TArray<ACharacterBase*>& PlayerCharacters, ETeamSideBase InTeamSide, float Distance);
	void DistributeExperience(ACharacterBase* Eliminator, const TArray<ACharacterBase*>& NearbyEnemies);

	// Animation Montages
	UFUNCTION(Server, Reliable)
	void StopAllMontages_Server(float BlendOut);
	UFUNCTION(NetMulticast, Reliable)
	void StopAllMontages_NetMulticast(float BlendOut);
	UFUNCTION(Server, Reliable)
	void PlayMontage_Server(UAnimMontage* Montage, float PlayRate);
	UFUNCTION(NetMulticast, Reliable)
	void PlayMontage_NetMulticast(UAnimMontage* Montage, float PlayRate);
	UFUNCTION(Server, Reliable)
	void MontageJumpToSection_Server(UAnimMontage* Montage, FName SectionName, float PlayRate);
	UFUNCTION(NetMulticast, Reliable)
	void MontageJumpToSection_NetMulticast(UAnimMontage* Montage, FName SectionName, float PlayRate);

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

	void SetAnimMontages(const TMap<FString, UAnimMontage*>& MontageMap);

	// Direction calculation
	UFUNCTION(BlueprintCallable, Category = "Direction")
	int32 GetRelativeDirection(AActor* OtherActor) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MinionBase")
	TObjectPtr<class UMinionAnimInstance> AnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MinionBase")
	TObjectPtr<class AMinionAIController> AIController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UCharacterWidgetComponent> WidgetComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "MinionBase", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_StateBar> StateBar;

public:
	UPROPERTY()
	EMinionType MinionType = EMinionType::None;

	UPROPERTY(ReplicatedUsing = OnRep_SkeletalMesh)
	USkeletalMesh* ReplicatedSkeletalMesh;

	// Experience sharing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExperienceShareRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, float> ShareFactor;

	// Animation Montages
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UAnimMontage*> Montages;

	// Bounty variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounty", meta = (AllowPrivateAccess = "true"))
	float ExpBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounty", meta = (AllowPrivateAccess = "true"))
	int32 GoldBounty;

	// Ragdoll and fade out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MinionBase", meta = (AllowPrivateAccess = "true"))
	float RagdollBlendTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MinionBase", meta = (AllowPrivateAccess = "true"))
	float ImpulseStrength = 1000.0f;

	FTimerHandle DeathMontageTimerHandle;
	FTimerHandle FadeOutTimerHandle;

	float CurrentFadeDeath = 0.f;
	float FadeOutDuration = 1.0f;
};
