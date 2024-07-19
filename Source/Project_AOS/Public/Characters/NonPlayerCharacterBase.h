// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "NonPlayerCharacterBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API ANonPlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

	friend class UBTTask_Attack;

public:
	ANonPlayerCharacterBase();

	bool GetIsAttacking() const { return bIsAttacking; }

	virtual void BeginPlay() override;

	virtual void SetWidget(class UUserWidgetBase* InUserWidgetBase) override;

	virtual void GetCrowdControl(EBaseCrowdControl InCondition = EBaseCrowdControl::None, float InDuration = 0.f, float InPercent = 0.f) override;

protected:
	UFUNCTION()
	virtual void Ability_Q() {};

	UFUNCTION()
	virtual void Ability_E() {};

	UFUNCTION()
	virtual void Ability_R() {};

	UFUNCTION()
	virtual void Ability_LMB() {};

	UFUNCTION()
	virtual void Ability_RMB() {};

	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bIsInterrupt) {};

	/* returns the Montage Section Attack1~3 */
	const FName GetAttackMontageSection(const int32& Section);

protected:
	/** Animinstance **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter")
	TObjectPtr<class UNPCAnimInstance> AnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter")
	TObjectPtr<class ANPCAIController> AIController;

	/**	 Animation Montages **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_Q_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_E_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_R_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_LMB_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_RMB_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Stun_Montage;

	/** NPC Wiget Components **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter", Meta = (AllowPrivateAccess))
	TObjectPtr<class UCharacterWidgetComponent> WidgetComponent;

	UPROPERTY(Meta = (AllowPrivateAccess))
	TArray<class AActor*> HitResults;

	UPROPERTY(Meta = (AllowPrivateAccess))
	TArray<FOverlapResult> OutHits;

	UPROPERTY()
	FTimerHandle CrowdControlTimer;

	uint8 CurrentComboCount : 2;
	uint8 MaxComboCount : 3;

	bool bIsAttacking;
};
