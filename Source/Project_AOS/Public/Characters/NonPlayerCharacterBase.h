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

protected:
	virtual void Ability_Q() override {};
	virtual void Ability_E() override {};
	virtual void Ability_R() override {};
	virtual void Ability_LMB() override {};
	virtual void Ability_RMB() override {};

	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bIsInterrupt) {};

	/* returns the Montage Section Attack1~3 */
	const FName GetAttackMontageSection(const int32& Section);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NonPlayerCharacter")
	TObjectPtr<class ANPCAIController> AIController;

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
