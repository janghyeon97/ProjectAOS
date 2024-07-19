// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

DECLARE_DELEGATE(FOnEnableSwitchActionNotifyBeginDelegate);
DECLARE_DELEGATE(FOnEnableMoveNotifyBeginDelegate);
DECLARE_DELEGATE(FOnCheckComboAttackNotifyBeginDelegate);
DECLARE_DELEGATE(FOnStopBasicAttackNotifyBeginDelegate);

UCLASS()
class PROJECT_AOS_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // Public methods
    void EnableTurnLeft();
    void EnableTurnRight();
    void OnTurnInPlaceEnded();
    void SetRootYawOffset(float DeltaSeconds);
    void SetTurningTime();
    void TurnInPlace(float DeltaSeconds);
    void PlayMontage(UAnimMontage* Montage, float PlayRate);

    void UpdateDeltaRotation();
    void SetTargetYawAndAnimLength(float AbsRootYawOffset);

    void SetTurnLeft(bool InTurnLeft) { bCanTurnLeft = InTurnLeft; }
    void SetTurnRight(bool InTurnRight) { bCanTurnRight = InTurnRight; }

    // Delegates
    FOnCheckComboAttackNotifyBeginDelegate OnCheckComboAttackNotifyBegin;
    FOnStopBasicAttackNotifyBeginDelegate OnStopBasicAttackNotifyBegin;
    FOnEnableMoveNotifyBeginDelegate OnEnableMoveNotifyBegin;
    FOnEnableSwitchActionNotifyBeginDelegate OnEnableSwitchActionNotifyBegin;

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    // AnimNotify functions
    UFUNCTION()
    void AnimNotify_CheckHit_Q();

    UFUNCTION()
    void AnimNotify_CheckHit_E();

    UFUNCTION()
    void AnimNotify_CheckHit_R();

    UFUNCTION()
    void AnimNotify_CheckHit_LMB();

    UFUNCTION()
    void AnimNotify_CheckHit_RMB();

    UFUNCTION()
    void AnimNotify_CanNextCombo();

    UFUNCTION()
    void AnimNotify_CanNextAction();

    UFUNCTION()
    void AnimNotify_CanMove();

    UFUNCTION()
    void AnimNotify_WeakAttackEnd();

    // Character References
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess))
    TObjectPtr<class AAOSCharacterBase> OwnerCharacter;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character", Meta = (AllowPrivateAccess))
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // Character Movement State
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Anim", meta = (AllowPrivateAccess))
    FVector MoveInputWithMaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Anim", meta = (AllowPrivateAccess))
    FVector MoveInput;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    float GroundSpeed;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    float YawOffset;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    float SpeedAccelDotProduct;

    // Character Aim Offset
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    FRotator BaseAimRotation;

    // Character Turn In Place
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    float RootYawOffset;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    bool bCanTurnLeft;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    bool bCanTurnRight;

    // Character States
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    uint8 bIsFalling : 1;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    uint8 bIsDead : 1;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character|Anim", Meta = (AllowPrivateAccess))
    bool bIsAbilityRActive = false;

    // Rotation and Timing Variables
    FRotator MovingRotation = FRotator::ZeroRotator;
    FRotator LastMovingRotation = FRotator::ZeroRotator;
    FRotator DeltaRotation = FRotator::ZeroRotator;
    FRotator LastDeltaRotation = FRotator::ZeroRotator;

    float LastInterpRotation = 0.0f;
    float TurningTimer = 0.0f;
    float RotationTimer = 0.0f;
    float TargetYaw = 0.0f;
    float AnimLength = 0.0f;

    bool bControlRotationStopped = false;
    bool bCanTurnInPlace = false;

    FTimerHandle TimerHandle;
};
