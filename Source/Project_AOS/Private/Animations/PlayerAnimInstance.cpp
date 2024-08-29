// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/PlayerAnimInstance.h"
#include "Characters/AOSCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
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
           /* Velocity = MovementComponent->Velocity;
            Acceleration = MovementComponent->GetCurrentAcceleration(); 
            Velocity.Normalize(0.0001);
            Acceleration.Normalize(0.0001);
            SpeedAccelDotProduct = FVector::DotProduct(Velocity, Acceleration);*/

            // YawOffset 계산
            YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(
                UKismetMathLibrary::MakeRotFromX(MovementComponent->Velocity),
                BaseAimRotation
            ).Yaw;


            ECharacterState NewCharacterState = ECharacterState::None;

            if (EnumHasAnyFlags(OwnerCharacter->CharacterState, EBaseCharacterState::Ability_Q))
            {
                EnumAddFlags(NewCharacterState, ECharacterState::Q);
            }

            if (EnumHasAnyFlags(OwnerCharacter->CharacterState, EBaseCharacterState::Ability_E))
            {
                EnumAddFlags(NewCharacterState, ECharacterState::E);
            }

            if (EnumHasAnyFlags(OwnerCharacter->CharacterState, EBaseCharacterState::Ability_R))
            {
                EnumAddFlags(NewCharacterState, ECharacterState::R);
            }

            if (EnumHasAnyFlags(OwnerCharacter->CharacterState, EBaseCharacterState::Ability_LMB))
            {
                EnumAddFlags(NewCharacterState, ECharacterState::LMB);
            }

            if (EnumHasAnyFlags(OwnerCharacter->CharacterState, EBaseCharacterState::Ability_RMB))
            {
                EnumAddFlags(NewCharacterState, ECharacterState::RMB);
            }

            // 상태가 변경되었을 때만 업데이트
            if (NewCharacterState != CharacterState)
            {
                CharacterState = NewCharacterState;

                FString CharacterStateString;
                UEnum* EnumPtr = StaticEnum<ECharacterState>();
                if (EnumPtr)
                {
                    for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
                    {
                        int64 Value = EnumPtr->GetValueByIndex(i);
                        if (EnumHasAnyFlags(CharacterState, static_cast<ECharacterState>(Value)) && Value != static_cast<uint8>(ECharacterState::None))
                        {
                            CharacterStateString += EnumPtr->GetNameStringByIndex(i) + TEXT(" ");
                        }
                    }
                    UE_LOG(LogTemp, Log, TEXT("[UPlayerAnimInstance::NativeUpdateAnimation] %s CharacterState: %s"), OwnerCharacter->HasAuthority() ? TEXT("Server") : TEXT("Client"), *CharacterStateString);
                }
            }
        }
    }
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float _DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(_DeltaSeconds);

    if (::IsValid(MovementComponent) && ::IsValid(OwnerCharacter))
    {
        FVector CurrentLocation = OwnerCharacter->GetActorLocation();
        FVector DisplacementVector = CurrentLocation - CurrentWorldLocation;
        DisplacementVector.Z = 0.0f;

        DisplacementSinceLastUpdate = DisplacementVector.Length();
        DisplacementSpeed = DisplacementSinceLastUpdate / _DeltaSeconds;

        CurrentWorldLocation = CurrentLocation;
        CurrentWorldRotation = OwnerCharacter->GetActorRotation();

        Acceleration = MovementComponent->GetCurrentAcceleration();
        Acceleration2D = FVector(Acceleration.X, Acceleration.Y, 0.0f);
        bHasAcceleration = !Acceleration2D.IsNearlyZero(0.0001);

        Velocity = MovementComponent->Velocity;
        Velocity2D = FVector(Velocity.X, Velocity.Y, 0.0f);
        LocomotionAngle = UKismetMathLibrary::NormalizedDeltaRotator(
            UKismetMathLibrary::MakeRotFromX(Velocity2D),
            OwnerCharacter->GetBaseAimRotation()
        ).Yaw;

        Velocity.Normalize(0.0001);
        Acceleration.Normalize(0.0001);
        SpeedAccelDotProduct = FVector::DotProduct(Velocity, Acceleration);

        Direction = Velocity2D.Length() > 0 ? CalculateLocomotionDirection(LocomotionAngle) : EDirection::None;
    }
}

void UPlayerAnimInstance::NativePostEvaluateAnimation()
{
    Super::NativePostEvaluateAnimation();
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

bool UPlayerAnimInstance::IsCharacterStateActive(ECharacterState State) const
{
    return EnumHasAnyFlags(CharacterState, State);
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

void UPlayerAnimInstance::AnimNotify_CheckHit_Q()
{
    if (::IsValid(OwnerCharacter) == false)
    {
        return;
    }

    OwnerCharacter->Ability_Q_CheckHit();
}

void UPlayerAnimInstance::AnimNotify_CheckHit_E()
{
    if (::IsValid(OwnerCharacter) == false)
    {
        return;
    }

    OwnerCharacter->Ability_E_CheckHit();
}

void UPlayerAnimInstance::AnimNotify_CheckHit_R()
{
    if (::IsValid(OwnerCharacter) == false)
    {
        return;
    }

    OwnerCharacter->Ability_R_CheckHit();
}

void UPlayerAnimInstance::AnimNotify_CheckHit_LMB()
{
    if (::IsValid(OwnerCharacter) == false)
    {
        return;
    }

    OwnerCharacter->Ability_LMB_CheckHit();
}

void UPlayerAnimInstance::AnimNotify_CheckHit_RMB()
{
    if (::IsValid(OwnerCharacter) == false)
    {
        return;
    }

    OwnerCharacter->Ability_RMB_CheckHit();
}

void UPlayerAnimInstance::AnimNotify_CanNextCombo()
{
	OnCheckComboAttackNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_CanNextAction()
{
	OnEnableSwitchActionNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_CanMove()
{
	OnEnableMoveNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_WeakAttackEnd()
{
	OnStopBasicAttackNotifyBegin.ExecuteIfBound();
}

void UPlayerAnimInstance::AnimNotify_EnableGravity()
{
    if (OnEnableGravityNotifyBegin.IsBound())
    {
        OnEnableGravityNotifyBegin.Broadcast();
    }
}

void UPlayerAnimInstance::AnimNotify_DisableGravity()
{
    if (OnDisableGravityNotifyBegin.IsBound())
    {
        OnDisableGravityNotifyBegin.Broadcast();
    }
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

UCharacterMovementComponent* UPlayerAnimInstance::GetMovementComponent() const
{
    ACharacter* Owner = Cast<ACharacter>(TryGetPawnOwner());
    if (!Owner)
    {
        return nullptr;
    }

    UCharacterMovementComponent* CharacterMovementComponent = Cast<UCharacterMovementComponent>(Owner->GetMovementComponent());
    if (!CharacterMovementComponent)
    {
        return nullptr;
    }

    return CharacterMovementComponent;
}

EDirection UPlayerAnimInstance::CalculateLocomotionDirection(const float InAngle) const
{
    const float RootAngle = FMath::Abs(InAngle);

    if (RootAngle <= 65.f)
    {
        return EDirection::Forward;
    }
    else
    {
        if (RootAngle >= 115.f)
        {
            return EDirection::Backward;
        }
        else
        {
            return InAngle >= 0 ? EDirection::Right : EDirection::Left;
        }
    }
}