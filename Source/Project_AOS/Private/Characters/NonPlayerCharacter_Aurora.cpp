// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NonPlayerCharacter_Aurora.h"
#include "Characters/AOSCharacterBase.h"
#include "Controllers/NPCAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animations/NPCAnimInstance.h"
#include "Components/StatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ANonPlayerCharacter_Aurora::ANonPlayerCharacter_Aurora()
{
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_Q_MONTAGE
	(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_Q_Montage.Ability_Q_Montage"));
	if (ABILITY_Q_MONTAGE.Succeeded())
		Ability_Q_Montage = ABILITY_Q_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_E_MONTAGE
	(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_E_Montage.Ability_E_Montage"));
	if (ABILITY_E_MONTAGE.Succeeded())
		Ability_E_Montage = ABILITY_E_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_R_MONTAGE
	(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_R_Montage.Ability_R_Montage"));
	if (ABILITY_R_MONTAGE.Succeeded())
		Ability_R_Montage = ABILITY_R_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_LMB_MONTAGE
	(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_LMB_Montage.Ability_LMB_Montage"));
	if (ABILITY_LMB_MONTAGE.Succeeded())
		Ability_LMB_Montage = ABILITY_LMB_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_RMB_MONTAGE
	(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
	if (ABILITY_RMB_MONTAGE.Succeeded())
		Ability_RMB_Montage = ABILITY_RMB_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> STUN_MONTAGE
	(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Stun_Montage.Stun_Montage"));
	if (STUN_MONTAGE.Succeeded())
		Stun_Montage = STUN_MONTAGE.Object;

	PrimaryActorTick.bCanEverTick = false;

	CurrentComboCount = 0;
	MaxComboCount = 4;
	bIsAttacking = false;
}

void ANonPlayerCharacter_Aurora::BeginPlay()
{
	Super::BeginPlay();

	if (::IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::MontageEnded);
		AnimInstance->OnNPCCanNextCombo.BindUObject(this, &ThisClass::CanNextCombo);
	}
}

void ANonPlayerCharacter_Aurora::Ability_Q()
{
}

void ANonPlayerCharacter_Aurora::Ability_E()
{
}

void ANonPlayerCharacter_Aurora::Ability_LMB()
{
	if (::IsValid(AnimInstance) && AnimInstance->Montage_IsPlaying(Ability_LMB_Montage) == false)
	{
		bIsAttacking = true;
		CurrentComboCount = 1;

		AnimInstance->PlayMontage(Ability_LMB_Montage);

		FVector CollisionBoxSize = FVector(200.0f, 100.0f, 150.0f);
		FVector CharacterForwadVector = GetActorForwardVector();
		FCollisionQueryParams params(NAME_None, false, this);

		bool bResult = GetWorld()->OverlapMultiByChannel(
			OutHits,
			GetActorLocation() + 120.f * CharacterForwadVector,
			FRotationMatrix::MakeFromZ(CharacterForwadVector).ToQuat(),
			ECC_GameTraceChannel3,
			FCollisionShape::MakeBox(CollisionBoxSize),
			params
		);

		if (bResult)
		{
			for (auto& OutHit : OutHits)
			{
				HitResults.AddUnique(OutHit.GetActor());
			}
		}

#pragma region CollisionDebugDrawing
		FVector TraceVec = CharacterForwadVector * 200;
		FVector Center = GetActorLocation() + 120.f * CharacterForwadVector;
		float HalfHeight = 100.f;
		FQuat BoxRot = FRotationMatrix::MakeFromZ(CharacterForwadVector).ToQuat();
		FColor DrawColor = true == bResult ? FColor::Green : FColor::Red;
		float DebugLifeTime = 5.f;

		DrawDebugBox(
			GetWorld(),
			Center,
			CollisionBoxSize,
			BoxRot,
			DrawColor,
			false,
			DebugLifeTime
		);
#pragma endregion
	}
}

void ANonPlayerCharacter_Aurora::CanNextCombo()
{
	bool bResult = false;

	UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(AIController->GetBlackboardComponent());

	if (::IsValid(AIController))
	{
		if (::IsValid(BlackboardComponent))
		{
			bResult = BlackboardComponent->GetValueAsBool(ANPCAIController::IsPlayerDetectedKey);
		}
	}

	if (bResult == true)
	{
		if (::IsValid(BlackboardComponent))
		{
			CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, MaxComboCount);

			AnimInstance->Montage_JumpToSection(GetAttackMontageSection(CurrentComboCount), Ability_LMB_Montage);
			BlackboardComponent->SetValueAsBool(ANPCAIController::IsPlayerDetectedKey, false);
		}
	}
}

void ANonPlayerCharacter_Aurora::OnPreDamageReceived(float FinalDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("ANonPlayerCharacter_Aurora::OnPreDamageReceived() FinalDamage: %f"), FinalDamage);
}

void ANonPlayerCharacter_Aurora::MontageEnded(UAnimMontage* Montage, bool bIsInterrupt)
{
	CurrentComboCount = 0;
	bIsAttacking = false;
}