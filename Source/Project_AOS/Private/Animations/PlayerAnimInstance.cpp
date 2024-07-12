// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/PlayerAnimInstance.h"
#include "Characters/AOSCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerAnimInstance::UPlayerAnimInstance()
{
	YawOffset = 0.f;
	RootYawOffset = 0.f;

	bIsFalling = false;
	bIsDead = false;
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AAOSCharacterBase>(GetOwningActor());
	if (::IsValid(OwnerCharacter))
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (::IsValid(OwnerCharacter))
    {
        if (!::IsValid(MovementComponent))
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }

        if (::IsValid(MovementComponent))
        {
            bIsFalling = MovementComponent->IsFalling();
            GroundSpeed = MovementComponent->GetLastUpdateVelocity().Size();
            SetRootYawOffset(DeltaSeconds);
            TurnInPlace(DeltaSeconds);

            // MoveInputWithMaxSpeed 계산
            float ForwardInputValue = FMath::Abs(MovementComponent->Velocity.X) * OwnerCharacter->GetForwardInputValue();
            float RightInputValue = FMath::Abs(MovementComponent->Velocity.Y) * OwnerCharacter->GetRightInputValue();
            float UpInputValue = MovementComponent->Velocity.Z;
            MoveInputWithMaxSpeed = FVector(ForwardInputValue, RightInputValue, UpInputValue);

            // MoveInput 정규화
            MoveInput.X = FMath::Abs(MoveInputWithMaxSpeed.X) < KINDA_SMALL_NUMBER ? 0.f : MoveInputWithMaxSpeed.X / FMath::Abs(MoveInputWithMaxSpeed.X);
            MoveInput.Y = FMath::Abs(MoveInputWithMaxSpeed.Y) < KINDA_SMALL_NUMBER ? 0.f : MoveInputWithMaxSpeed.Y / FMath::Abs(MoveInputWithMaxSpeed.Y);
            MoveInput.Z = FMath::Abs(MoveInputWithMaxSpeed.Z) < KINDA_SMALL_NUMBER ? 0.f : MoveInputWithMaxSpeed.Z / FMath::Abs(MoveInputWithMaxSpeed.Z);

            // 조준 회전 업데이트
            BaseAimRotation.Pitch = OwnerCharacter->GetAimPitchValue();
            BaseAimRotation.Yaw = OwnerCharacter->GetAimYawValue();

            // 속도와 가속도의 내적을 계산하여 저장합니다.
            FVector Velocity = MovementComponent->Velocity;
            FVector Acceleration = MovementComponent->GetCurrentAcceleration();
            Velocity.Normalize(0.0001);
            Acceleration.Normalize(0.0001);
            SpeedAccelDotProduct = FVector::DotProduct(Velocity, Acceleration);

            // YawOffset 계산
            YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(
                UKismetMathLibrary::MakeRotFromX(MovementComponent->Velocity),
                BaseAimRotation
            ).Yaw;

            // 능력 상태 업데이트
            bIsAbilityRActive = EnumHasAnyFlags(OwnerCharacter->CharacterState, EBaseCharacterState::Ability_R);
        }
    }
}

void UPlayerAnimInstance::SetRootYawOffset(float DeltaSeconds)
{
    if (GroundSpeed > 0.f || bIsFalling)
    {
        // RootYawOffset를 0으로 보간합니다.
        RootYawOffset = FMath::FInterpTo(RootYawOffset, 0, DeltaSeconds, 20.f);
        MovingRotation = OwnerCharacter->GetActorRotation();
        LastMovingRotation = MovingRotation;
        bCanTurnInPlace = false;
        RotationTimer = 0.f;
    }
    else
    {
        UpdateDeltaRotation();

        if (bControlRotationStopped)
        {
            TurningTimer += DeltaSeconds;
            if (TurningTimer > 0.5f)
            {
                SetTurningTime();
            }
        }
    }
}

void UPlayerAnimInstance::UpdateDeltaRotation()
{
    LastMovingRotation = MovingRotation;
    LastDeltaRotation = DeltaRotation;

    MovingRotation = OwnerCharacter->GetActorRotation();
    DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(MovingRotation, LastMovingRotation);

    RootYawOffset -= DeltaRotation.Yaw;

    if (DeltaRotation.Yaw == 0 && FMath::Abs(LastDeltaRotation.Yaw) > 0)
    {
        bControlRotationStopped = true;
    }
    else if (FMath::Abs(DeltaRotation.Yaw) > 0 && FMath::Abs(LastDeltaRotation.Yaw) > 0)
    {
        TurningTimer = 0.f;
        bControlRotationStopped = false;
    }
}

void UPlayerAnimInstance::SetTurningTime()
{
    float AbsRootYawOffset = FMath::Abs(RootYawOffset);

    if (AbsRootYawOffset >= 90.f)
    {
        TurningTimer = 0.f;
        LastInterpRotation = 0.f;
        bControlRotationStopped = false;
        bCanTurnInPlace = true;

        if (RootYawOffset > 0)
        {
            bCanTurnLeft = true;
            SetTargetYawAndAnimLength(AbsRootYawOffset);
        }
        else
        {
            bCanTurnRight = true;
            SetTargetYawAndAnimLength(AbsRootYawOffset);
        }
    }
    else
    {
        TurningTimer = 0.f;
        bControlRotationStopped = false;
    }
}

void UPlayerAnimInstance::SetTargetYawAndAnimLength(float AbsRootYawOffset)
{
    if (AbsRootYawOffset >= 180.f)
    {
        TargetYaw = 180.f;
        AnimLength = 1.5f;
    }
    else if (AbsRootYawOffset < 180.f && AbsRootYawOffset > 90.f)
    {
        TargetYaw = 90.f;
        AnimLength = 1.3f;
    }
}

void UPlayerAnimInstance::TurnInPlace(float DeltaSeconds)
{
    if (bCanTurnInPlace)
    {
        RotationTimer += DeltaSeconds;

        float InterpolationAlpha = FMath::Clamp(RotationTimer / AnimLength, 0.f, 1.f);
        float InterpCircularInOut = FMath::InterpSinInOut(0.f, TargetYaw, InterpolationAlpha);

        float DeltaInterp = InterpCircularInOut - LastInterpRotation;

        RootYawOffset += (RootYawOffset > 0) ? -DeltaInterp : DeltaInterp;

        LastInterpRotation = InterpCircularInOut;

        if (InterpolationAlpha >= 1.f)
        {
            RotationTimer = 0.f;
            bCanTurnLeft = false;
            bCanTurnRight = false;
            bCanTurnInPlace = false;
        }
    }
}

void UPlayerAnimInstance::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage_IsPlaying(Montage))
	{
		//UE_LOG(LogTemp, Log, TEXT("Play %s"), *Montage->GetName());

		Montage_Play(Montage, PlayRate, EMontagePlayReturnType::Duration, 0.0f, true);
	}
}

void UPlayerAnimInstance::AnimNotify_CanNextCombo()
{
	OnCheckComboAttackNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_CanNextAction()
{
	OnEnableSwithActionNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_CanMove()
{
	OnEnableMoveNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_WeakAttackEnd()
{
	OnStopBasicAttackNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_DashEnded()
{
	OnAuroraDashEnded.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_SpawnParticle()
{
	OnSpawnParticleNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::EnableTurnLeft()
{
	bCanTurnLeft = true;
}

void UPlayerAnimInstance::EnableTurnRight()
{
	bCanTurnRight = true;
}

void UPlayerAnimInstance::OnTurnInPlaceEnded()
{
	bCanTurnLeft = false;
	bCanTurnRight = false;
}
