// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/MinionAnimInstance.h"
#include "Characters/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMinionAnimInstance::NativeInitializeAnimation()
{
	bIsDead = false;

	OwnerCharacter = Cast<ACharacterBase>(TryGetPawnOwner());
}

void UMinionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (::IsValid(OwnerCharacter) == true)
	{
		UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
		if (::IsValid(CharacterMovementComponent) == true)
		{
			Velocity = CharacterMovementComponent->GetLastUpdateVelocity();
			Acceleration = CharacterMovementComponent->GetCurrentAcceleration();
			CurrentSpeed = Velocity.Size();
			bIsFalling = CharacterMovementComponent->IsFalling();
			bShouldMove = (!Acceleration.Equals(FVector(0.f, 0.f, 0.f)) && CurrentSpeed > 3.f) ? true : false;
			bIsAccelerating = Acceleration.Length() > 0 ? true : false;

			bIsDead = EnumHasAnyFlags(OwnerCharacter->CharacterState, EBaseCharacterState::Death);
		}
	}
}

void UMinionAnimInstance::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage_IsPlaying(Montage))
	{
		Montage_Play(Montage, PlayRate, EMontagePlayReturnType::Duration, 0.0f, true);
	}
}