// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/AuroraCharacter.h"
#include "Animations/PlayerAnimInstance.h"
#include "Components/StatComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/InputConfigData.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Structs/DamageInfomationStruct.h"
#include "Props/SplineActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


AAuroraCharacter::AAuroraCharacter()
{
	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	AbilityStatComponent = CreateDefaultSubobject<UAbilityStatComponent>(TEXT("AbilityStatComponent"));

	if (HasAuthority())
	{
		StatComponent->SetIsReplicated(true);
		AbilityStatComponent->SetIsReplicated(true);
	}

	InitializeAbilityMontages();
	InitializeAbilityParticles();
	InitializeAbilityMeshes();

	Ability_E_FirstDelay = 0.2f;
	Ability_E_BoostStrength = 10;
	Ability_E_ShieldDuration = 1.25f;

	Ability_R_BoostStrength = 600.f;
	Ability_R_ExplodeDelay = 0.67f;
	Ability_R_StunDuration = 2.0f;
	Ability_R_Range = 600.f;

	Ability_LMB_CurrentComboCount = 0;
	Ability_LMB_MaxComboCount = 4;

	Ability_RMB_LastInterp = 0;
	Ability_RMB_QuadraticScale = 3.f;
	Ability_RMB_JumpScale = 60.f;

	SplinePointIndex = 0;

	bIsTumbling = false;

	SelectedCharacterIndex = 1;

	PrimaryActorTick.bCanEverTick = true;
}

void AAuroraCharacter::InitializeAbilityMontages()
{
	Super::InitializeAbilityMontages();

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_Q_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_Q_Montage.Ability_Q_Montage"));
	if (ABILITY_Q_MONTAGE.Succeeded()) Ability_Q_Montage = ABILITY_Q_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_E_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_E_Montage.Ability_E_Montage"));
	if (ABILITY_E_MONTAGE.Succeeded()) Ability_E_Montage = ABILITY_E_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_R_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_R_Montage.Ability_R_Montage"));
	if (ABILITY_R_MONTAGE.Succeeded()) Ability_R_Montage = ABILITY_R_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_LMB_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_LMB_Montage.Ability_LMB_Montage"));
	if (ABILITY_LMB_MONTAGE.Succeeded()) Ability_LMB_Montage = ABILITY_LMB_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_RMB_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
	if (ABILITY_RMB_MONTAGE.Succeeded()) Ability_RMB_Montage = ABILITY_RMB_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> STUN_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Stun_Montage.Stun_Montage"));
	if (STUN_MONTAGE.Succeeded()) Stun_Montage = STUN_MONTAGE.Object;
}

void AAuroraCharacter::InitializeAbilityParticles()
{
	Super::InitializeAbilityParticles();

	static ConstructorHelpers::FObjectFinder<UParticleSystem> MELEE_SUCCESS_IMPACT(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Primary/FX/P_Aurora_Melee_SucessfulImpact.P_Aurora_Melee_SucessfulImpact"));
	if (MELEE_SUCCESS_IMPACT.Succeeded()) MeleeSuccessImpact = MELEE_SUCCESS_IMPACT.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_ROOTED(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Rooted.P_Aurora_Freeze_Rooted"));
	if (ABILITY_Q_ROOTED.Succeeded()) FreezeRooted = ABILITY_Q_ROOTED.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_SEGMENT(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Segment.P_Aurora_Freeze_Segment"));
	if (ABILITY_Q_SEGMENT.Succeeded()) FreezeSegment = ABILITY_Q_SEGMENT.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_WHRILWIND(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Whrilwind.P_Aurora_Freeze_Whrilwind"));
	if (ABILITY_Q_WHRILWIND.Succeeded()) FreezeWhrilwind = ABILITY_Q_WHRILWIND.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_SEGMENTtCRUMBLE(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Segment_Crumble.P_Aurora_Freeze_Segment_Crumble"));
	if (ABILITY_Q_SEGMENTtCRUMBLE.Succeeded()) SegmentCrumble = ABILITY_Q_SEGMENTtCRUMBLE.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> SCREEN_FROST_FROZEN(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_ScreenFrost_Frozen.P_Aurora_ScreenFrost_Frozen"));
	if (SCREEN_FROST_FROZEN.Succeeded()) ScreenFrostFrozen = SCREEN_FROST_FROZEN.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ULTIMATE_EXPLODE(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Explode.P_Aurora_Ultimate_Explode"));
	if (ULTIMATE_EXPLODE.Succeeded()) UltimateExplode = ULTIMATE_EXPLODE.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ULTIMATE_WARMUP(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Warmup.P_Aurora_Ultimate_Warmup"));
	if (ULTIMATE_WARMUP.Succeeded()) UltimateWarmUp = ULTIMATE_WARMUP.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ULTIMATE_FROZEN(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Frozen.P_Aurora_Ultimate_Frozen"));
	if (ULTIMATE_FROZEN.Succeeded()) UltimateFrozen = ULTIMATE_FROZEN.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ULTIMATE_INITIALBLAST(TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_InitialBlast.P_Aurora_Ultimate_InitialBlast"));
	if (ULTIMATE_INITIALBLAST.Succeeded()) UltimateInitialBlast = ULTIMATE_INITIALBLAST.Object;
}

void AAuroraCharacter::InitializeAbilityMeshes()
{
	Super::InitializeAbilityMeshes();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ABILITY_E_SHIELDBOTTOM(TEXT("/Game/Paragon/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Bottom.SM_FrostShield_Spikey_Bottom"));
	if (ABILITY_E_SHIELDBOTTOM.Succeeded()) ShieldBottom = ABILITY_E_SHIELDBOTTOM.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ABILITY_E_SHIELDMIDDLE(TEXT("/Game/Paragon/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Middle.SM_FrostShield_Spikey_Middle"));
	if (ABILITY_E_SHIELDMIDDLE.Succeeded()) ShieldMiddle = ABILITY_E_SHIELDMIDDLE.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ABILITY_E_SHIELDTOP(TEXT("/Game/Paragon/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Top.SM_FrostShield_Spikey_Top"));
	if (ABILITY_E_SHIELDTOP.Succeeded()) ShieldTop = ABILITY_E_SHIELDTOP.Object;
}

void AAuroraCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 서버 권한이 있고, 캐릭터가 텀블링 중일 때의 처리
	if (HasAuthority() && bIsTumbling)
	{
		HandleTumbling(DeltaSeconds);
	}

	// 클라이언트에서 부드러운 이동 처리를 위한 처리
	if (!HasAuthority() && bSmoothMovement)
	{
		SmoothMovement(DeltaSeconds);
	}
}

void AAuroraCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (::IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AAuroraCharacter::MontageEnded);
		AnimInstance->OnStopBasicAttackNotifyBegin.BindUObject(this, &AAuroraCharacter::Ability_LMB_AttackEnded);
		AnimInstance->OnAuroraDashEnded.BindUObject(this, &AAuroraCharacter::Ability_E_Ended);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::BeginPlay] AnimInstance is no vaild."));
	}
}

void AAuroraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
		{
			if (::IsValid(EnhancedInputComponent))
			{
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Triggered, this, &AAuroraCharacter::Move);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &AAuroraCharacter::Look);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &AAuroraCharacter::Jump);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_Q_Action, ETriggerEvent::Started, this, &AAuroraCharacter::Ability_Q);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_E_Action, ETriggerEvent::Started, this, &AAuroraCharacter::Ability_E);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_R_Action, ETriggerEvent::Started, this, &AAuroraCharacter::Ability_R);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_LMB_Action, ETriggerEvent::Started, this, &AAuroraCharacter::Ability_LMB);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_RMB_Action, ETriggerEvent::Started, this, &AAuroraCharacter::Ability_RMB);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->CallAFunctionAction, ETriggerEvent::Started, this, &AAuroraCharacter::ExecuteSomethingSpecial);
			}
		}
	));
}

void AAuroraCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ReplicatedTargetLocation);
	DOREPLIFETIME(ThisClass, bSmoothMovement);
}

void AAuroraCharacter::Move(const FInputActionValue& InValue)
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Move))
	{
		if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::AbilityUsed))
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::AbilityUsed);
			AnimInstance->StopAllMontages(0.25f);
			StopAllMontages_Server(0.25f);
		}

		PreviousForwardInputValue = ForwardInputValue;
		PreviousRightInputValue = RightInputValue;

		ForwardInputValue = InValue.Get<FVector2D>().X;
		RightInputValue = InValue.Get<FVector2D>().Y;

		const FRotator ControlRotation = GetController()->GetControlRotation();
		const FRotator ControlRotationYaw(0.f, ControlRotation.Yaw, 0.f);

		const FVector ForwardVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
		const FVector RightVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardVector, ForwardInputValue);
		AddMovementInput(RightVector, RightInputValue);
	}
}

void AAuroraCharacter::Look(const FInputActionValue& InValue)
{
	FVector2D LookVector = InValue.Get<FVector2D>();
	FRotator AimRotation = GetBaseAimRotation();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);

	CurrentAimYaw = AimRotation.Yaw;
	CurrentAimPitch = AimRotation.Pitch;

	UpdateAimValue_Server(CurrentAimPitch, CurrentAimYaw);
}

/*
	1. UniqueValue[0]: RootDuration			중앙 파티클 지속시간
	2. UniqueValue[1]: RingDuration			링 파티클 지속시간
	3. UniqueValue[2]: Radius				링 반경
*/
void AAuroraCharacter::Ability_Q()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Dead))
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_Q] Ability cannot be used because the Ctrl key is pressed."));
		return;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(AbilityStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_Q] StatComponent, AbilityStatComponent, or AnimInstance is null."));
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_Q);
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) && bAbilityReady)
	{
		//AbilityStatComponent->UseAbility(EAbilityID::Ability_Q, GetWorld()->GetTimeSeconds());
		//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_Q);

		EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);

		// 현재 캐릭터의 위치, 회전, 방향 벡터를 저장
		LastCharacterLocation = GetActorLocation();
		LastCharacterRotation = GetActorRotation();
		LastForwardVector = GetActorForwardVector();
		LastRightVector = GetActorRightVector();
		LastUpVector = GetActorUpVector();

		// 능력 통계 값을 가져와서 고유 값을 확인
		TArray<float> UniqueValues = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_Q, 0).UniqueValue;;
		FTransform Transform(FRotator(0), LastCharacterLocation + FVector(0, 0, -95.f), FVector(1));
		FCircularParticleInfomation CircularParticleInfomation;

		Ability_Q_RingDuration = 1.f;
		Ability_Q_Radius = 10.f;

		// 고유 값이 유효한 경우 능력 값을 업데이트
		if (UniqueValues.IsValidIndex(1) && UniqueValues.IsValidIndex(2))
		{
			Ability_Q_RingDuration = UniqueValues[1];
			Ability_Q_Radius = UniqueValues[2];
		}

		Ability_Q_Rate = 0.01f;
		Ability_Q_FirstDelay = 0.35f;
		Ability_Q_NumParicles = 28;
		Ability_Q_ParticleScale = 1;

		// 변환 설정 업데이트
		Transform.SetRotation(LastCharacterRotation.Quaternion());
		Transform.SetScale3D(FVector(Ability_Q_ParticleScale));

		CircularParticleInfomation.ForwardVector = LastForwardVector;
		CircularParticleInfomation.RightVector = LastRightVector;
		CircularParticleInfomation.UpVector = LastUpVector;
		CircularParticleInfomation.NumOfParicles = Ability_Q_NumParicles;
		CircularParticleInfomation.Angle = 360.f / Ability_Q_NumParicles;
		CircularParticleInfomation.Radius = Ability_Q_Radius;
		CircularParticleInfomation.Lifetime = Ability_Q_RingDuration;
		CircularParticleInfomation.Transform = Transform;
		CircularParticleInfomation.Rate = Ability_Q_Rate;
		CircularParticleInfomation.Delay = Ability_Q_FirstDelay;

		AnimInstance->PlayMontage(Ability_Q_Montage, 1.0f);
		PlayMontage_Server(Ability_Q_Montage, 1.0f);

		if (::IsValid(FreezeRooted) && ::IsValid(FreezeWhrilwind) && ::IsValid(FreezeSegment))
		{
			SpawnRootedParicleAtLocation_Server(FreezeRooted, Transform);
			SpawnRootedParicleAtLocation_Server(FreezeWhrilwind, Transform);
			SpawnCircularParicleAtLocation_Server(FreezeSegment, CircularParticleInfomation, Transform);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_Q] FreezeRooted, FreezeWhrilwind, or FreezeSegment particles are not valid."));
			return;
		}

		Ability_Q_CheckHit();

	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}

void AAuroraCharacter::Ability_E()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Dead))
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_E] Ability cannot be used because the Ctrl key is pressed."));
		return;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(AbilityStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_E] StatComponent, AbilityStatComponent, or AnimInstance is null."));
		return;
	}


	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_E);
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) && bAbilityReady)
	{
		AbilityStatComponent->UseAbility(EAbilityID::Ability_E, GetWorld()->GetTimeSeconds());

		uint8 InstanceIndex = AbilityStatComponent->GetAbilityInfomation(EAbilityID::Ability_E).InstanceIndex;
		if (InstanceIndex == 1)
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
			EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
			EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);

			bUseControllerRotationYaw = false;

			LastCharacterLocation = GetActorLocation() + FVector(0, 0, -95.f);
			LastCharacterRotation = GetActorRotation();
			LastForwardVector = GetActorForwardVector();

			UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
			if (MovementComponent)
			{
				MovementComponent->MaxWalkSpeed = 2000.f;
				MovementComponent->MinAnalogWalkSpeed = 1000.f;
				MovementComponent->MaxAcceleration = 100000.f;
			}

			Ability_E_Started_Server();

			if (::IsValid(Ability_E_Montage))
			{
				AnimInstance->PlayMontage(Ability_E_Montage, 1.0f);
				PlayMontage_Server(Ability_E_Montage, 1.0f);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_E] Ability_E_Montage is null."));
				return;
			}

			GetWorldTimerManager().SetTimer(
				Ability_E_Timer,
				FTimerDelegate::CreateLambda([this]()
					{
						AddMovementInput(LastForwardVector, Ability_E_BoostStrength);
					}),
				0.01f,
				true,
				Ability_E_FirstDelay
			);

			if (::IsValid(ShieldTop) && ::IsValid(ShieldMiddle) && ::IsValid(ShieldBottom))
			{
				SpawnAttachedMeshAtLocation_Server(ShieldBottom, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, -50.0f), Ability_E_ShieldDuration);
				SpawnAttachedMeshAtLocation_Server(ShieldMiddle, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, 0.0f), Ability_E_ShieldDuration);
				SpawnAttachedMeshAtLocation_Server(ShieldTop, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, 50.0f), Ability_E_ShieldDuration);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_E] ShieldTop, ShieldMiddle or ShieldBottom is null."));
			}

			Ability_E_CheckHit();
		}
		else
		{
			AbilityStatComponent->UseAbility(EAbilityID::Ability_E, GetWorld()->GetTimeSeconds());
		}
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}

void AAuroraCharacter::Ability_R()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Dead))
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_R] Ability cannot be used because the Ctrl key is pressed."));
		return;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(AbilityStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_R] StatComponent, AbilityStatComponent, or AnimInstance is null."));
		return;
	}


	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_R);
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) && bAbilityReady)
	{
		AbilityStatComponent->UseAbility(EAbilityID::Ability_R, GetWorld()->GetTimeSeconds());
		AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_R);

		EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);

		LastCharacterLocation = GetActorLocation();

		if (::IsValid(Ability_R_Montage))
		{
			AnimInstance->PlayMontage(Ability_R_Montage, 1.0f);
			PlayMontage_Server(Ability_R_Montage, 1.0f);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ASparrowCharacter::Ability_R] Ability_R_Montage is null."));
			return;
		}

		Ability_R_Started_Server();

		if (::IsValid(UltimateWarmUp))
		{
			SpawnAttachedParicleAtLocation_Server(UltimateWarmUp, FTransform(FRotator(0), LastCharacterLocation, FVector(1)));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ASparrowCharacter::Ability_R] UltimateWarmUp particle is not valid, unable to spawn particle."));
		}

		GetWorldTimerManager().SetTimer(
			Ability_R_Timer,
			FTimerDelegate::CreateLambda([this]()
				{
					if (UltimateExplode)
					{
						SpawnRootedParicleAtLocation_Server(UltimateExplode, FTransform(FRotator(0), GetActorLocation(), FVector(1)));
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("[ASparrowCharacter::Ability_R] UltimateExplode particle is not valid, unable to spawn particle."));
					}
					Ability_R_CheckHit();
				}),
			Ability_R_ExplodeDelay,
			false
		);
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}

/**
 * @brief LMB 능력을 활성화합니다. Ctrl 키가 눌리거나 AbilityStatComponent가 없는 경우 능력을 사용할 수 없습니다.
 *        능력이 준비된 경우, 능력을 사용하고 쿨다운을 시작합니다. 첫 번째 공격과 후속 공격을 처리합니다.
 */
void AAuroraCharacter::Ability_LMB()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Dead))
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_LMB] Ability cannot be used because the Ctrl key is pressed."));
		return;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(AbilityStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_LMB] StatComponent, AbilityStatComponent, or AnimInstance is null."));
		return;
	}


	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_LMB);
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) && bAbilityReady)
	{
		AbilityStatComponent->UseAbility(EAbilityID::Ability_LMB, GetWorld()->GetTimeSeconds());
		AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_LMB);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);

		if (::IsValid(Ability_LMB_Montage) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_LMB] Ability_LMB_Montage is null."));
			return;
		}

		if (Ability_LMB_CurrentComboCount == 0)
		{
			StartComboAttack();
		}
		else
		{
			ContinueComboAttack();
		}
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}

/**
 * LMB 콤보의 첫 번째 공격을 시작합니다. 애니메이션 재생 속도를 설정하고 몽타주를 재생합니다.
 */
void AAuroraCharacter::StartComboAttack()
{
	Ability_LMB_CurrentComboCount = 1;
	Ability_LMB_AnimLength = 1.0f;

	Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);
	AnimInstance->PlayMontage(Ability_LMB_Montage, Ability_LMB_PlayRate);
	PlayMontage_Server(Ability_LMB_Montage, Ability_LMB_PlayRate);
}

/**
 * LMB 콤보의 후속 공격을 처리합니다. 현재 콤보 횟수를 업데이트하고 다음 공격 섹션으로 이동합니다.
 */
void AAuroraCharacter::ContinueComboAttack()
{
	Ability_LMB_CurrentComboCount = FMath::Clamp<int32>((Ability_LMB_CurrentComboCount % 4) + 1, 1, Ability_LMB_MaxComboCount);

	switch (Ability_LMB_CurrentComboCount)
	{
	case 1:
	case 2:
	case 3:
		Ability_LMB_AnimLength = 1.0f;
		break;
	case 4:
		Ability_LMB_AnimLength = 1.7f;
		break;
	default:
		Ability_LMB_AnimLength = 1.0f;
		break;
	}

	Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);
	FName NextSectionName = GetAttackMontageSection(Ability_LMB_CurrentComboCount);

	AnimInstance->Montage_SetPlayRate(Ability_LMB_Montage, Ability_LMB_PlayRate);
	AnimInstance->PlayMontage(Ability_LMB_Montage, Ability_LMB_PlayRate);
	AnimInstance->Montage_JumpToSection(NextSectionName, Ability_LMB_Montage);

	PlayMontage_Server(Ability_LMB_Montage, Ability_LMB_PlayRate);
	MontageJumpToSection_Server(Ability_LMB_Montage, NextSectionName, Ability_LMB_PlayRate);
}


/**
 * RMB 능력을 활성화합니다. Ctrl 키가 눌렸는지 여부를 확인하고, 능력이 준비되었는지 확인합니다.
 * 능력이 준비되었으면 이동 및 행동 상태를 업데이트하고, 해당 섹션의 애니메이션을 재생한 후 서버에서 능력을 실행합니다.
 *
	- UniqueValue[0]: Range			사거리
 */
void AAuroraCharacter::Ability_RMB()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Dead))
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_RMB] Ability cannot be used because the Ctrl key is pressed."));
		return;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(AbilityStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_RMB] StatComponent, AbilityStatComponent, or AnimInstance is null."));
		return;
	}


	bool bIsAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_RMB);

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) && bIsAbilityReady)
	{
		// 능력을 사용하고 쿨다운 시작
		//AbilityStatComponent->UseAbility(EAbilityID::Ability_RMB, GetWorld()->GetTimeSeconds());
		//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_RMB);

		EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);

		const FRotator CurrentControlRotation = GetController()->GetControlRotation();
		const FRotator ControlRotationYaw(0.f, CurrentControlRotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);
		const FVector MoveDirection = (ForwardDirection * ForwardInputValue) + (RightDirection * RightInputValue);

		int32 DirectionIndex = CalculateDirectionIndex();
		FName MontageSectionName = FName(*FString::Printf(TEXT("RMB%d"), DirectionIndex));

		if (::IsValid(Ability_RMB_Montage))
		{
			AnimInstance->PlayMontage(Ability_RMB_Montage, 1.0f);
			AnimInstance->Montage_JumpToSection(MontageSectionName, Ability_RMB_Montage);
			PlayMontage_Server(Ability_RMB_Montage, 1.0f);
			MontageJumpToSection_Server(Ability_RMB_Montage, MontageSectionName, 1.0f);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_RMB] Ability_RMB_Montage is null."));
		}

		Ability_RMB_Server(MoveDirection);
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}

/**
 * 서버에서 RMB 능력을 실행합니다. 캐릭터의 현재 위치와 이동 방향을 기반으로 목표 위치를 설정합니다.
 * 목표 위치로의 이동 중 장애물 충돌 여부를 확인하고, 충돌한 경우 적절히 처리합니다.
 */
void AAuroraCharacter::Ability_RMB_Server_Implementation(FVector MoveDirection)
{
	if (::IsValid(AbilityStatComponent) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_RMB_Server] AbilityStatComponent is not valid."));
		return;
	}

	TArray<FAbilityStatTable> AbilityStats = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_RMB);

	if (AbilityStats.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_RMB_Server] AbilityStats array is empty."));
		return;
	}

	TArray<float> UniqueValues = AbilityStats[0].UniqueValue;
	if (UniqueValues.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_RMB_Server] UniqueValue array is empty."));
		return;
	}

	// UniqueValue 배열의 요소가 유효한지 검사
	for (int32 i = 0; i < UniqueValues.Num(); i++)
	{
		if (UniqueValues.IsValidIndex(i) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_RMB_Server] There is an invalid value in the UniqueValues array at index: %d"), i);
			return;
		}
	}

	LastCharacterLocation = GetActorLocation() - FVector(0, 0, 45.f);
	LastForwardVector = GetActorForwardVector();

	FVector CurrentLocation = GetActorLocation() - FVector(0, 0, 45.f);
	FVector TargetLocation = CurrentLocation + (MoveDirection * UniqueValues[0]);

	FCollisionQueryParams CollisionParams(NAME_None, false, this);
	FHitResult FirstHitResult;
	bool bHitOccurred = GetWorld()->LineTraceSingleByChannel(
		FirstHitResult,
		CurrentLocation,
		TargetLocation,
		ECC_Visibility,
		CollisionParams
	);

	DrawDebugLine(GetWorld(), CurrentLocation, TargetLocation, bHitOccurred ? FColor::Magenta : FColor::Red, false, 5.0f, 0, 2.0f);

	if (bHitOccurred)
	{
		FVector AdjustedLocation = FirstHitResult.Location + MoveDirection * 10;

		FHitResult SecondHitResult;
		bool bSecondHitOccurred = GetWorld()->LineTraceSingleByChannel(
			SecondHitResult,
			AdjustedLocation + FVector(0, 0, 1000.f),
			AdjustedLocation + FVector(0, 0, -1000.f),
			ECC_Visibility,
			CollisionParams
		);

		DrawDebugLine(GetWorld(), AdjustedLocation + FVector(0, 0, 1000), AdjustedLocation + FVector(0, 0, -1000), bSecondHitOccurred ? FColor::Orange : FColor::Red, false, 5.0f, 0, 2.0f);

		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::ExecuteAbilityRMB_Server] Distance %f"), FirstHitResult.Location.Z - SecondHitResult.Location.Z);
		if (FMath::Abs(FirstHitResult.Location.Z - SecondHitResult.Location.Z) >= 500.f)
		{
			TargetLocation = (AdjustedLocation - MoveDirection * 10.f) + FVector(0, 0, -90.f);
		}
		else
		{
			TargetLocation = SecondHitResult.Location + MoveDirection * 50;
		}
	}
	else
	{
		bool bSecondHitOccurred = GetWorld()->LineTraceSingleByChannel(
			FirstHitResult,
			TargetLocation + FVector(0, 0, 300.f),
			TargetLocation + FVector(0, 0, -1000.f),
			ECC_Visibility,
			CollisionParams
		);

		DrawDebugLine(GetWorld(), TargetLocation + FVector(0, 0, 300.f), TargetLocation + FVector(0, 0, -1000), bSecondHitOccurred ? FColor::Emerald : FColor::Red, false, 5.0f, 0, 2.0f);

		TargetLocation = FirstHitResult.Location;
	}

	Ability_RMB_TargetLocation = TargetLocation;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	Ability_RMB_ElapsedTime = 0.f;
	bIsTumbling = true;
	bSmoothMovement = true;
}

/**
 * 텀블링 중인 캐릭터를 처리합니다. 일정 시간 동안 텀블링 상태를 유지하고, 텀블링이 끝나면 상태를 업데이트합니다.
 * @param DeltaSeconds 프레임 간의 시간 간격
 */
void AAuroraCharacter::HandleTumbling(float DeltaSeconds)
{
	Ability_RMB_ElapsedTime += DeltaSeconds;

	float InterpolationAlpha = FMath::Clamp(Ability_RMB_ElapsedTime / 0.35f, 0.0f, 1.0f);
	if (InterpolationAlpha >= 1.0f)
	{
		bIsTumbling = false;
		bSmoothMovement = false;
	}
	else
	{
		FVector CurrentLocation = FMath::Lerp(LastCharacterLocation, Ability_RMB_TargetLocation, InterpolationAlpha);
		float ParabolicHeight = FMath::Sin(InterpolationAlpha * PI) * 500;  // 높이를 조정
		float HeightAdjustment = Ability_RMB_TargetLocation.Z - LastCharacterLocation.Z;

		CurrentLocation.Z += ParabolicHeight + HeightAdjustment * InterpolationAlpha;  // 목표 위치의 높이에 따라 높이를 조정

		bool bLocationSet = SetActorLocation(CurrentLocation, true, nullptr, ETeleportType::None);
		if (!bLocationSet)
		{
			bSmoothMovement = false;
		}

		DrawDebugLine(GetWorld(), GetActorLocation(), CurrentLocation, bLocationSet ? FColor::Green : FColor::Red, false, 5.0f, 0, 2.0f);

		ReplicatedTargetLocation = CurrentLocation;
	}
}

/**
 * 캐릭터를 목표 위치로 부드럽게 이동시킵니다.
 * 현재 위치와 목표 위치 사이를 보간하여 캐릭터를 이동시킵니다.
 */
void AAuroraCharacter::SmoothMovement(float DeltaSeconds)
{
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, ReplicatedTargetLocation, DeltaSeconds, 20.f);
	SetActorLocation(NewLocation);
}

/**
 * 이동 방향을 기반으로 방향 인덱스를 계산합니다.
 * 이동 벡터와 캐릭터의 기준 조준 방향을 사용하여 각도를 계산하고, 이를 8방향(각 방향은 45도 간격) 중 하나로 매핑합니다.
 * @param MoveDirection 이동 방향 벡터
 * @return 1에서 8까지의 방향 인덱스
 */
int32 AAuroraCharacter::CalculateDirectionIndex()
{
	float Yaw = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::MakeRotFromX(GetCharacterMovement()->Velocity),
		GetBaseAimRotation()
	).Yaw;

	float NormalizedYaw = FMath::Fmod(Yaw + 360.0f, 360.0f);
	int32 DirectionIndex = FMath::RoundToInt(NormalizedYaw / 45.0f) % 8;

	return DirectionIndex + 1;
}

void AAuroraCharacter::Ability_Q_CheckHit()
{
	float CollisionSphereSize = Ability_Q_Radius; // y x z
	FCollisionQueryParams params(NAME_None, false, this);

	OutHits.Empty();

	bool bResult = GetWorld()->OverlapMultiByChannel(
		OutHits,
		GetActorLocation() + FVector(0, 0, -45.f),
		FQuat::Identity,
		ECC_GameTraceChannel3,
		FCollisionShape::MakeSphere(CollisionSphereSize),
		params
	);

	if (bResult)
	{
		for (const auto& OutHit : OutHits)
		{
			if (::IsValid(OutHit.GetActor()))
			{
				ACharacterBase* Character = Cast<ACharacterBase>(OutHit.GetActor());
				if (::IsValid(Character))
				{
					// 적팀일 경우에만 데미지 적용.
					if (Character->TeamSide != this->TeamSide)
					{
						const FAbilityStatTable AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_Q, 0);

						const float Character_AttackDamage = StatComponent->GetAttackDamage();
						const float Character_AbilityPower = StatComponent->GetAbilityPower();

						const float BaseAttackDamage = AbilityStatTable.Ability_AttackDamage;
						const float BaseAbilityPower = AbilityStatTable.Ability_AbilityPower;
						const float Ability_AD_Ratio = AbilityStatTable.Ability_AD_Ratio;
						const float Ability_AP_Ratio = AbilityStatTable.Ability_AP_Ratio;

						const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * Ability_AD_Ratio) + (BaseAbilityPower + Character_AbilityPower * Ability_AP_Ratio);

						FDamageInfomation DamageInfomation;
						DamageInfomation.AbilityID = EAbilityID::Ability_Q;
						DamageInfomation.AttackEffect = EAttackEffect::AbilityEffects;
						DamageInfomation.AbilityEffects.Add(ECrowdControlBase::Slow);
						DamageInfomation.AddDamage(EDamageType::Magic, FinalDamage);

						ApplyDamage_Server(Character, DamageInfomation, GetController(), this);
					}
				}
			}
		}

	}
}

void AAuroraCharacter::Ability_E_CheckHit()
{

}

void AAuroraCharacter::Ability_R_CheckHit()
{
	float CollisionSphereSize = Ability_R_Range; // y x z
	FCollisionQueryParams params(NAME_None, false, this);

	OutHits.Empty();

	bool bResult = GetWorld()->OverlapMultiByChannel(
		OutHits,
		GetActorLocation() + FVector(0, 0, -45.f),
		FQuat::Identity,
		ECC_GameTraceChannel3,
		FCollisionShape::MakeSphere(CollisionSphereSize),
		params
	);

	if (bResult)
	{
		for (const auto& OutHit : OutHits)
		{
			if (::IsValid(OutHit.GetActor()))
			{
				ACharacterBase* Character = Cast<ACharacterBase>(OutHit.GetActor());
				if (::IsValid(Character))
				{
					// 적팀일 경우에만 데미지 적용.
					if (Character->TeamSide != this->TeamSide)
					{
						const FAbilityStatTable AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_R, 0);

						const float Character_AttackDamage = StatComponent->GetAttackDamage();
						const float Character_AbilityPower = StatComponent->GetAbilityPower();

						const float BaseAttackDamage = AbilityStatTable.Ability_AttackDamage;
						const float BaseAbilityPower = AbilityStatTable.Ability_AbilityPower;
						const float Ability_AD_Ratio = AbilityStatTable.Ability_AD_Ratio;
						const float Ability_AP_Ratio = AbilityStatTable.Ability_AP_Ratio;

						const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * Ability_AD_Ratio) + (BaseAbilityPower + Character_AbilityPower * Ability_AP_Ratio);

						FDamageInfomation DamageInfomation;
						DamageInfomation.AbilityID = EAbilityID::Ability_R;
						DamageInfomation.AttackEffect = EAttackEffect::AbilityEffects;
						DamageInfomation.AbilityEffects.Add(ECrowdControlBase::Stun);
						DamageInfomation.AddDamage(EDamageType::Magic, FinalDamage);

						ApplyDamage_Server(Character, DamageInfomation, GetController(), this);
					}
				}
			}
		}
	}

#pragma region CollisionDebugDrawing
	FColor DrawColor = true == bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.f;

	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		CollisionSphereSize,
		16,
		DrawColor,
		false,
		DebugLifeTime
	);
#pragma endregion
}

void AAuroraCharacter::Ability_LMB_CheckHit()
{
	if (::IsValid(this))
	{
		FVector CollisionBoxSize = FVector(150.0f, 100.0f, 110.0f);
		FVector CharacterForwadVector = GetActorForwardVector();
		FCollisionQueryParams params(NAME_None, false, this);

		OutHits.Empty();

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
			for (const auto& OutHit : OutHits)
			{
				if (::IsValid(OutHit.GetActor()) == false)
				{
					continue;
				}

				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor Name: %s"), *OutHit.GetActor()->GetName()));

				ACharacterBase* Character = Cast<ACharacterBase>(OutHit.GetActor());
				if (::IsValid(Character) == false)
				{
					return;
				}

				// 적팀일 경우에만 데미지 적용.
				if (Character->TeamSide != this->TeamSide)
				{
					const FAbilityStatTable AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB, 0);

					const float Character_AttackDamage = StatComponent->GetAttackDamage();
					const float Character_AbilityPower = StatComponent->GetAbilityPower();

					const float BaseAttackDamage = AbilityStatTable.Ability_AttackDamage;
					const float BaseAbilityPower = AbilityStatTable.Ability_AbilityPower;
					const float Ability_AD_Ratio = AbilityStatTable.Ability_AD_Ratio;
					const float Ability_AP_Ratio = AbilityStatTable.Ability_AP_Ratio;

					const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * Ability_AD_Ratio) + (BaseAbilityPower + Character_AbilityPower * Ability_AP_Ratio);

					FDamageInfomation DamageInfomation;
					DamageInfomation.AbilityID = EAbilityID::Ability_LMB;
					EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::OnHit);
					EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::OnAttack);
					DamageInfomation.AddDamage(EDamageType::Physical, FinalDamage);

					ApplyDamage_Server(Character, DamageInfomation, GetController(), this);

					/* Spawn HitSuccessImpact */
					FTransform transform(FRotator(0), Character->GetMesh()->GetSocketLocation("Impact"), FVector(1));
					SpawnRootedParicleAtLocation_Server(MeleeSuccessImpact, transform);
				}
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

void AAuroraCharacter::Ability_RMB_CheckHit()
{
	OutHits.Empty();
}

void AAuroraCharacter::OnPreDamageReceived(float FinalDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("AAuroraCharacter::OnPreDamageReceived() FinalDamage: %f"), FinalDamage);
}

void AAuroraCharacter::ExecuteSomethingSpecial()
{
	GetCrowdControl(ECrowdControlBase::Blind, 2.0);
}

void AAuroraCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == Ability_LMB_Montage)
	{

	}
	else if (Montage == Ability_E_Montage)
	{
		GetWorldTimerManager().ClearTimer(Ability_E_Timer);
		GetCharacterMovement()->MaxWalkSpeed = StatComponent->GetMovementSpeed();
		GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
		GetCharacterMovement()->MaxAcceleration = 2048.f;
		GetCharacterMovement()->GravityScale = 1.6f;
		bUseControllerRotationYaw = true;
	}
	else
	{
		EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::AbilityUsed);
	}
}

void AAuroraCharacter::Ability_LMB_AttackEnded()
{
	Ability_LMB_CurrentComboCount = 0;
}

void AAuroraCharacter::Ability_E_Ended()
{
	AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_E);

	Ability_E_Ended_Server();
	GetWorldTimerManager().ClearTimer(Ability_E_Timer);
	GetCharacterMovement()->MaxWalkSpeed = StatComponent->GetMovementSpeed();
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->MaxAcceleration = 2048.f;
	GetCharacterMovement()->GravityScale = 1.6f;
}

void AAuroraCharacter::Ability_E_Started_Server_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 2000.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 1000.f;
	GetCharacterMovement()->MaxAcceleration = 100000.f;
}

void AAuroraCharacter::Ability_E_Ended_Server_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = StatComponent->GetMovementSpeed();
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->MaxAcceleration = 2048.f;
	GetCharacterMovement()->GravityScale = 1.6f;
}

void AAuroraCharacter::Ability_R_Started_Server_Implementation()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		LaunchCharacter(FVector(0, 0, Ability_R_BoostStrength), false, true);
	}
	else
	{
		LaunchCharacter(FVector(0, 0, Ability_R_BoostStrength / 3), false, true);
	}
}