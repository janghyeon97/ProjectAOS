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
#include "Structs/CustomCombatData.h"
#include "Game/AOSGameInstance.h"
#include "Props/SplineActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Props/FreezeSegment.h"
#include "CrowdControls/StunEffect.h"
#include "CrowdControls/SlowEffect.h"


AAuroraCharacter::AAuroraCharacter()
{
	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	AbilityStatComponent = CreateDefaultSubobject<UAbilityStatComponent>(TEXT("AbilityStatComponent"));

	if (HasAuthority())
	{
		StatComponent->SetIsReplicated(true);
		AbilityStatComponent->SetIsReplicated(true);
	}

	bIsTumbling = false;
	bIsDashing = false;

	Ability_LMB_CurrentComboCount = 0;
	Ability_LMB_MaxComboCount = 4;
		
	SelectedCharacterIndex = 1;
	ChampionName = "Aurora";

	PrimaryActorTick.bCanEverTick = true;

	InitializeCharacterResources();
}

void AAuroraCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 서버 권한이 있고, 캐릭터가 텀블링 중일 때의 처리
	if (HasAuthority() && bIsTumbling)
	{
		HandleTumbling(DeltaSeconds);
	}

	// 서버 권한이 있고, 캐릭터가 대쉬 중일 때의 처리
	if (HasAuthority() && bIsDashing)
	{
		HandleDashing(DeltaSeconds);
	}

	// 클라이언트에서 부드러운 이동 처리를 위한 처리
	if (!HasAuthority() && bSmoothMovement)
	{
		SmoothMovement(DeltaSeconds);
	}

	FVector DebugLoation = GetActorLocation() - FVector(0, 0, 95.f);
	FVector DebugForwardVector = GetActorForwardVector();
	DrawDebugDirectionalArrow(GetWorld(), DebugLoation, DebugLoation + DebugForwardVector * 200, 5, FColor::Red, false, -1.0f, 0, 1);
}

void AAuroraCharacter::BeginPlay()
{
	Super::BeginPlay();

	UPlayerAnimInstance* PlayerAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(PlayerAnimInstance))
	{
		PlayerAnimInstance->OnMontageEnded.AddDynamic(this, &AAuroraCharacter::MontageEnded);
		PlayerAnimInstance->OnStopBasicAttackNotifyBegin.BindUObject(this, &AAuroraCharacter::Ability_LMB_AttackEnded);
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
	if (!EnumHasAnyFlags(CharacterState, EBaseCharacterState::Move) || EnumHasAnyFlags(CrowdControlState, EBaseCrowdControl::Stun) || EnumHasAnyFlags(CrowdControlState, EBaseCrowdControl::Snare))
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::AbilityUsed))
	{
		EnumRemoveFlags(CharacterState, EBaseCharacterState::AbilityUsed);
		ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::AbilityUsed);

		AnimInstance->StopAllMontages(0.5f);
		StopAllMontages_Server(0.5, false);
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Recall))
	{
		ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Recall);

		AnimInstance->StopAllMontages(0.1f);
		StopAllMontages_Server(0.1, false);
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
	3. UniqueValue[2]: ParticleScale		링 파티클 크기
	4. UniqueValue[3]: NumParticles			링 파티클 개수
	5. UniqueValue[4]: FirstDelay			링 파티클 초기 지연시간
	6. UniqueValue[5]: Rate					링 파티클 생성 속도
*/
void AAuroraCharacter::Ability_Q()
{
	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bIsAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_Q);
	if (bIsAbilityReady)
	{
		ServerNotifyAbilityUse(EAbilityID::Ability_Q, ETriggerEvent::Started);
		SaveCharacterTransform();

		const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_Q);

		const float Ability_Q_Radius = AbilityStatTable.Radius;
		const float Ability_Q_RingDuration = GetUniqueAttribute(EAbilityID::Ability_Q, "RingDuration", 2.f);
		const float Ability_Q_ParticleScale = GetUniqueAttribute(EAbilityID::Ability_Q, "ParticleScale", 1.f);
		const float Ability_Q_NumParicles = GetUniqueAttribute(EAbilityID::Ability_Q, "NumParticles", 28.f);
		const float Ability_Q_FirstDelay = GetUniqueAttribute(EAbilityID::Ability_Q, "FirstDelay", 0.35f);
		const float Ability_Q_Rate = GetUniqueAttribute(EAbilityID::Ability_Q, "Rate", 0.01f);

		FCachedParticleInfo ParticleInfomation;
		ParticleInfomation.Lifetime = Ability_Q_RingDuration;
		ParticleInfomation.NumParicles = Ability_Q_NumParicles;
		ParticleInfomation.Scale = Ability_Q_ParticleScale;
		ParticleInfomation.Rate = Ability_Q_Rate;
		ParticleInfomation.Radius = Ability_Q_Radius;

		const float Character_AttackDamage = StatComponent->GetAttackDamage();
		const float Character_AbilityPower = StatComponent->GetAbilityPower();

		const float BaseAttackDamage = AbilityStatTable.AttackDamage;
		const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
		const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
		const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

		const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

		FDamageInformation DamageInformation;
		DamageInformation.AbilityID = EAbilityID::Ability_Q;
		DamageInformation.AttackEffect = EAttackEffect::AbilityEffects;
		DamageInformation.AddDamage(EDamageType::Magic, FinalDamage);

		FTransform Transform(UKismetMathLibrary::MakeRotFromX(LastForwardVector), LastCharacterLocation, FVector(1));

		PlayMontage("Q", 1.0, NAME_None, TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_Q_Montage.Ability_Q_Montage"));
		SpawnFreezeSegments_Server(Transform, ParticleInfomation, DamageInformation);
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}




/*
 *	Ability_E 함수는 E 능력을 활성화합니다.
 *	능력 사용 가능 여부와 준비 상태를 확인한 후, 캐릭터를 목표 위치로 대쉬하고 보호막 메쉬를 생성합니다.
 *	고유 속성 값을 가져와서 능력의 범위와 지속 시간을 설정합니다.
 *	생성된 목표 위치는 서버로 전달되어 서버에서 이동이 처리됩니다.
 *
 *	1. UniqueValue[0]: Duration			얼음길 지속시간
 */
void AAuroraCharacter::Ability_E()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_E);
	if (!bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	//AbilityStatComponent->UseAbility(EAbilityID::Ability_E, GetWorld()->GetTimeSeconds());
	//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_E);

	ServerNotifyAbilityUse(EAbilityID::Ability_E, ETriggerEvent::None);
	SaveCharacterTransform();

	bUseControllerRotationYaw = false;

	float Range = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_E).Range;
		FVector TargetLocation = LastCharacterLocation + LastForwardVector * (Range > 0 ? Range : 900.f);

	UStaticMesh* ShieldTop = GetOrLoadMesh(TEXT("ShieldTop"), TEXT("/Game/Paragon/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Top.SM_FrostShield_Spikey_Top"));
	UStaticMesh* ShieldMiddle = GetOrLoadMesh(TEXT("ShieldMiddle"), TEXT("/Game/Paragon/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Middle.SM_FrostShield_Spikey_Middle"));
	UStaticMesh* ShieldBottom = GetOrLoadMesh(TEXT("ShieldBottom"), TEXT("/Game/Paragon/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Bottom.SM_FrostShield_Spikey_Bottom"));

	if (::IsValid(ShieldTop) && ::IsValid(ShieldMiddle) && ::IsValid(ShieldBottom))
	{
		SpawnAttachedMeshAtLocation_Server(ShieldBottom, LastCharacterLocation * 10, 1.25f);
		SpawnAttachedMeshAtLocation_Server(ShieldMiddle, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, 90.0f), 1.25f);
		SpawnAttachedMeshAtLocation_Server(ShieldTop, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, 180.0f), 1.25f);
	}

	Ability_E_Server(TargetLocation);
	PlayMontage("E", 1.0, NAME_None, TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_E_Montage.Ability_E_Montage"));
}

void AAuroraCharacter::Ability_E_Server_Implementation(FVector TargetLocation)
{
	SaveCharacterTransform();

	Ability_E_TargetLocation = TargetLocation;

	DrawDebugSphere(GetWorld(), TargetLocation, 10.f, 32, FColor::Red, false, 5.0f, 0, 2.0f);

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	Ability_ElapsedTime = 0.f;
	bIsDashing = true;
	bSmoothMovement = true;
}

void AAuroraCharacter::HandleDashing(float DeltaSeconds)
{
	Ability_ElapsedTime += DeltaSeconds;

	float InterpolationAlpha = FMath::Clamp(Ability_ElapsedTime / 0.5f, 0.0f, 1.0f);
	if (InterpolationAlpha >= 1.0f)
	{
		bIsDashing = false;
		bSmoothMovement = false;
	}
	else
	{
		FVector NewLocation = FMath::Lerp(LastCharacterLocation, Ability_E_TargetLocation, InterpolationAlpha) + FVector(0, 0, 95.f);

		bool bLocationSet = SetActorLocation(NewLocation, true, nullptr, ETeleportType::None);
		if (!bLocationSet)
		{
			bSmoothMovement = false;
		}

		DrawDebugSphere(GetWorld(), NewLocation, 5.f, 32, FColor::Emerald, false, 5.0f, 0, 2.0f);

		ReplicatedTargetLocation = NewLocation;
	}
}


/*
	1. UniqueValue[0]: HeroShatterAbilityDamage			챔피언 대상 추가 데미지
	2. UniqueValue[1]: NonHeroShatterAbilityDamage		비 챔피언 대상 추가 데미지
	3. UniqueValue[2]: InitialPowerScaling				초기 AbilityPower 계수
	4. UniqueValue[3]: PowerScalingOnHero				챔피언 대상 AbilityPower 계수
	5. UniqueValue[4]: PowerScalingOnNonHero			비 챔피언 대상 AbilityPower 계수
	6. UniqueValue[5]: ChainRadius						연쇄 폭발 사거리
	7. UniqueValue[6]: SlowDuration						이동속도 감소 지속시간
	8. UniqueValue[7]: MovementSpeedSlow				이동속도 감소율
	9. UniqueValue[8]: StunDuration						기절 지속시간
	10. UniqueValue[9]: ExplodeTime						연쇄 폭팔 시간
	10. UniqueValue[9]: BoostStrength					점프 강도
*/
void AAuroraCharacter::Ability_R()
{
	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_R);
	if (!bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	//AbilityStatComponent->UseAbility(EAbilityID::Ability_R, GetWorld()->GetTimeSeconds());
	//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_R);

	const float BoostStrength = GetUniqueAttribute(EAbilityID::Ability_R, TEXT("BoostStrength"), 600.f);

	ServerNotifyAbilityUse(EAbilityID::Ability_R, ETriggerEvent::None);
	SaveCharacterTransform();

	PlayMontage("R", 1.0, NAME_None, TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_R_Montage.Ability_R_Montage"));
	Ability_R_Started_Server(BoostStrength);
}

void AAuroraCharacter::Ability_R_Started_Server_Implementation(const float BoostStrength)
{
	if (!GetCharacterMovement()->IsFalling())
	{
		LaunchCharacter(FVector(0, 0, BoostStrength), false, true);
	}
	else
	{
		LaunchCharacter(FVector(0, 0, BoostStrength / 3), false, true);
	}
}

/**
 * @brief LMB 능력을 활성화합니다. Ctrl 키가 눌리거나 AbilityStatComponent가 없는 경우 능력을 사용할 수 없습니다.
 *        능력이 준비된 경우, 능력을 사용하고 쿨다운을 시작합니다. 첫 번째 공격과 후속 공격을 처리합니다.
 */
void AAuroraCharacter::Ability_LMB()
{
	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_LMB);
	if (bAbilityReady)
	{
		AbilityStatComponent->UseAbility(EAbilityID::Ability_LMB, GetWorld()->GetTimeSeconds());
		AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_LMB);

		ServerNotifyAbilityUse(EAbilityID::Ability_LMB, ETriggerEvent::None);

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

	PlayMontage(TEXT("LMB"), Ability_LMB_PlayRate, NAME_None, TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_LMB_Montage.Ability_LMB_Montage"));
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
		Ability_LMB_AnimLength = 0.5f;
		break;
	default:
		Ability_LMB_AnimLength = 1.0f;
		break;
	}

	Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);

	FName NextSectionName = GetAttackMontageSection(Ability_LMB_CurrentComboCount);
	PlayMontage("LMB", Ability_LMB_PlayRate, NextSectionName, TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_LMB_Montage.Ability_LMB_Montage"));
}



/**
 * RMB 능력을 활성화합니다. Ctrl 키가 눌렸는지 여부를 확인하고, 능력이 준비되었는지 확인합니다.
 * 능력이 준비되었으면 이동 및 행동 상태를 업데이트하고, 해당 섹션의 애니메이션을 재생한 후 서버에서 능력을 실행합니다.
 *
 * 캐릭터의 현재 위치와 이동 방향을 기반으로 목표 위치를 설정합니다.
 * 목표 위치로의 이동 중 장애물 충돌 여부를 확인하고, 충돌한 경우 적절히 처리합니다.
 *
 *	- UniqueValue[0]: Range			사거리
 */
void AAuroraCharacter::Ability_RMB()
{
	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bIsAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_RMB);
	if (bIsAbilityReady)
	{
		// 능력을 사용하고 쿨다운 시작
		//AbilityStatComponent->UseAbility(EAbilityID::Ability_RMB, GetWorld()->GetTimeSeconds());
		//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_RMB);		

		ServerNotifyAbilityUse(EAbilityID::Ability_RMB, ETriggerEvent::None);
		SaveCharacterTransform();

		const FRotator CurrentControlRotation = GetController()->GetControlRotation();
		const FRotator ControlRotationYaw(0.f, CurrentControlRotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);
		const FVector MoveDirection = (ForwardDirection * ForwardInputValue) + (RightDirection * RightInputValue);

		int32 DirectionIndex = CalculateDirectionIndex();
		FName MontageSectionName = FName(*FString::Printf(TEXT("RMB%d"), DirectionIndex));

		PlayMontage("RMB", 1.0f, MontageSectionName, TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
		Ability_RMB_Server(MoveDirection);
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}


FVector AAuroraCharacter::CalculateTargetLocation(const FVector& MoveDirection, const float Range)
{
	FVector CurrentLocation = GetActorLocation() - FVector(0, 0, 45.f);
	FVector TargetLocation = CurrentLocation + (MoveDirection * Range);

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

	return TargetLocation;
}

bool AAuroraCharacter::Ability_RMB_Server_Validate(FVector MoveDirection)
{
	// 서버에서 검증 로직 추가
	// TargetLocation이 합리적인 범위 내에 있는지 확인합니다.

	return true;
}

void AAuroraCharacter::Ability_RMB_Server_Implementation(FVector MoveDirection)
{
	if (!IsValid(AbilityStatComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_RMB_Server] AbilityStatComponent is not valid."));
		return;
	}

	FAbilityStatTable AbilityStats = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_RMB);

	if (AbilityStats.Range <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_RMB_Server] Invalid ability range: %f"), AbilityStats.Range);
		return;
	}

	if (MoveDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_RMB_Server] MoveDirection is nearly zero."));
		return;
	}

	float Range = AbilityStats.Range;

	SaveCharacterTransform();

	Ability_RMB_TargetLocation = CalculateTargetLocation(MoveDirection, Range);

	// Check if the calculated target location is valid
	if (!Ability_RMB_TargetLocation.ContainsNaN() && Ability_RMB_TargetLocation != FVector::ZeroVector)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		Ability_ElapsedTime = 0.f;
		bIsTumbling = true;
		bSmoothMovement = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::Ability_RMB_Server] Calculated target location is invalid: %s"), *Ability_RMB_TargetLocation.ToString());
	}
}

/**
 * 텀블링 중인 캐릭터를 처리합니다. 일정 시간 동안 텀블링 상태를 유지하고, 텀블링이 끝나면 상태를 업데이트합니다.
 * @param DeltaSeconds 프레임 간의 시간 간격
 */
void AAuroraCharacter::HandleTumbling(float DeltaSeconds)
{
	Ability_ElapsedTime += DeltaSeconds;

	float InterpolationAlpha = FMath::Clamp(Ability_ElapsedTime / 0.35f, 0.0f, 1.0f);
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

/*
 *	1. UniqueValue[0]: HeroShatterAbilityDamage			챔피언 대상 추가 데미지
 *	2. UniqueValue[1]: NonHeroShatterAbilityDamage		비 챔피언 대상 추가 데미지
 *	3. UniqueValue[2]: InitialPowerScaling				초기 AbilityPower 계수
 *	4. UniqueValue[3]: PowerScalingOnHero				챔피언 대상 AbilityPower 계수
 *	5. UniqueValue[4]: PowerScalingOnNonHero			비 챔피언 대상 AbilityPower 계수
 *	6. UniqueValue[5]: ChainRadius						연쇄 폭발 사거리
 *	7. UniqueValue[6]: SlowDuration						이동속도 감소 지속시간
 *	8. UniqueValue[7]: MovementSpeedSlow				이동속도 감소율
 *	9. UniqueValue[8]: StunDuration						기절 지속시간
 *	10. UniqueValue[9]: ExplodeTime						연쇄 폭팔 시간
 */
void AAuroraCharacter::Ability_R_CheckHit()
{
	const FAbilityStatTable& StatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_R);

	float CollisionSphereSize = StatTable.Radius;
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

	if (!bResult)
	{
		return;
	}

	TArray<ACharacterBase*> FrozenEnemies;

	for (const auto& OutHit : OutHits)
	{
		if (!::IsValid(OutHit.GetActor()))
		{
			continue;
		}

		ACharacterBase* Character = Cast<ACharacterBase>(OutHit.GetActor());
		if (!::IsValid(Character))
		{
			continue;
		}

		// 적 팀일 경우에만 데미지 적용
		if (Character->TeamSide != this->TeamSide)
		{
			const float SlowDuration = GetUniqueAttribute(EAbilityID::Ability_R, "SlowDuration", 1.5f);
			const float MovementSpeedSlow = GetUniqueAttribute(EAbilityID::Ability_R, "MovementSpeedSlow", 20.f);

			UParticleSystem* UltimateSlowed = GetOrLoadParticle(TEXT("UltimateSlowed"), TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Slowed.P_Aurora_Ultimate_Slowed"));
			if (UltimateSlowed)
			{
				SpawnAttachedParticleAtLocation_Server(UltimateSlowed, Character->GetMesh(), FTransform(FRotator(0), GetMesh()->GetSocketLocation(FName("Root")), FVector(1)), EAttachLocation::KeepWorldPosition);
			}

			FCrowdControlInformation CrowdControlInformation;
			CrowdControlInformation.Type = EBaseCrowdControl::Slow;
			CrowdControlInformation.Duration = SlowDuration;
			CrowdControlInformation.Percent = MovementSpeedSlow;

			ApplyCrowdControl_Server(Character, CrowdControlInformation);
			
			FrozenEnemies.Add(Character);
		}
	}

	const float ExplodeTime = GetUniqueAttribute(EAbilityID::Ability_R, "ExplodeTime", 2.0f);
	const int32 TimerID = static_cast<uint32>(EAbilityID::Ability_R);

	// 폭발 타이머 설정
	auto TimerCallback = [this, TimerID, StatTable, FrozenEnemies]()
		{
			// 데미지 계산
			const float Character_AttackDamage = StatComponent->GetAttackDamage();
			const float Character_AbilityPower = StatComponent->GetAbilityPower();

			const float BaseAttackDamage = StatTable.AttackDamage;
			const float BaseAbilityDamage = StatTable.AbilityDamage;
			const float AD_PowerScaling = StatTable.AD_PowerScaling;
			const float AP_PowerScaling = StatTable.AP_PowerScaling;

			const float HeroShatterAbilityDamage = GetUniqueAttribute(EAbilityID::Ability_R, "HeroShatterAbilityDamage", 0.f);
			const float NonHeroShatterAbilityDamage = GetUniqueAttribute(EAbilityID::Ability_R, "NonHeroShatterAbilityDamage", 0.f);
			const float InitialPowerScaling = GetUniqueAttribute(EAbilityID::Ability_R, "InitialPowerScaling", 0.f);
			const float PowerScalingOnHero = GetUniqueAttribute(EAbilityID::Ability_R, "PowerScalingOnHero", 0.f);
			const float PowerScalingOnNonHero = GetUniqueAttribute(EAbilityID::Ability_R, "PowerScalingOnNonHero", 0.f); 
			const float ChainRadius = GetUniqueAttribute(EAbilityID::Ability_R, "ChainRadius", 500.f);
			const float StunDuration = GetUniqueAttribute(EAbilityID::Ability_R, "StunDuration", 1.f);

			float FinalDamage = 0;

			for (auto& FrozenEnemy : FrozenEnemies)
			{
				if (!::IsValid(FrozenEnemy)) continue;

				FinalDamage = BaseAbilityDamage + Character_AbilityPower * InitialPowerScaling;

				FDamageInformation DamageInformation;
				DamageInformation.AbilityID = EAbilityID::Ability_R;
				DamageInformation.AddDamage(EDamageType::Magic, FinalDamage);
				

				ApplyDamage_Server(FrozenEnemy, DamageInformation, GetController(), this);

				UParticleSystem* UltimateExplode = GetOrLoadParticle(TEXT("UltimateExplode"), TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Explode.P_Aurora_Ultimate_Explode"));
				UParticleSystem* UltimateFrozen = GetOrLoadParticle(TEXT("UltimateFrozen"), TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Frozen.P_Aurora_Ultimate_Frozen"));

				if (UltimateExplode) SpawnAttachedParticleAtLocation_Server(UltimateExplode, FrozenEnemy->GetMesh(), FrozenEnemy->GetMesh()->GetSocketTransform(FName("Root")), EAttachLocation::KeepWorldPosition);
				if (UltimateFrozen)	SpawnAttachedParticleAtLocation_Server(UltimateFrozen, FrozenEnemy->GetMesh(), FrozenEnemy->GetMesh()->GetSocketTransform(FName("Root")), EAttachLocation::KeepWorldPosition);

				// 연쇄 폭발 처리
				TArray<FOverlapResult> ChainHits;
				FCollisionQueryParams ChainParams(NAME_None, false, this);
				GetWorld()->OverlapMultiByChannel(
					ChainHits,
					FrozenEnemy->GetActorLocation(),
					FQuat::Identity,
					ECC_GameTraceChannel3,
					FCollisionShape::MakeSphere(ChainRadius),
					ChainParams
				);

				for (const auto& ChainHit : ChainHits)
				{
					ACharacterBase* ChainCharacter = Cast<ACharacterBase>(ChainHit.GetActor());
					if (::IsValid(ChainCharacter) && ChainCharacter->TeamSide != this->TeamSide && !FrozenEnemies.Contains(ChainCharacter))
					{
						if (EnumHasAnyFlags(FrozenEnemy->ObjectType, EObjectType::Minion))
						{
							FinalDamage = (BaseAbilityDamage + Character_AbilityPower * PowerScalingOnNonHero) + NonHeroShatterAbilityDamage;
						}
						else if (EnumHasAnyFlags(FrozenEnemy->ObjectType, EObjectType::Player))
						{
							FinalDamage = (BaseAbilityDamage + Character_AbilityPower * PowerScalingOnHero) + HeroShatterAbilityDamage;
						}

						FDamageInformation ChainDamageInfomation;
						ChainDamageInfomation.AbilityID = EAbilityID::Ability_R;
						ChainDamageInfomation.AddDamage(EDamageType::Magic, FinalDamage);

						ApplyDamage_Server(FrozenEnemy, DamageInformation, GetController(), this);
					}
				}
			}
		};

	SetGameTimer(AbilityTimer, TimerID, TimerCallback, 1.0, false, ExplodeTime);

#pragma region CollisionDebugDrawing
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
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
	if (!::IsValid(this) || !::IsValid(CurrentTarget))
	{
		return;
	}

	ACharacterBase* TargetCharacter = Cast<ACharacterBase>(CurrentTarget);
	if (!::IsValid(TargetCharacter))
	{
		return;
	}

	// 적팀일 경우에만 데미지 적용.
	if (TargetCharacter->TeamSide != this->TeamSide)
	{
		const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB);

		const float Character_AttackDamage = StatComponent->GetAttackDamage();
		const float Character_AbilityPower = StatComponent->GetAbilityPower();

		const float BaseAttackDamage = AbilityStatTable.AttackDamage;
		const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
		const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
		const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

		const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

		FDamageInformation DamageInformation;
		DamageInformation.AbilityID = EAbilityID::Ability_LMB;
		EnumAddFlags(DamageInformation.AttackEffect, EAttackEffect::OnHit);
		EnumAddFlags(DamageInformation.AttackEffect, EAttackEffect::OnAttack);
		DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);

		ApplyDamage_Server(TargetCharacter, DamageInformation, GetController(), this);

		/* Spawn HitSuccessImpact */
		UParticleSystem* MeleeSuccessImpact = GetOrLoadParticle("MeleeSuccessImpact", TEXT("/Game/Paragon/ParagonAurora/FX/Particles/Abilities/Primary/FX/P_Aurora_Melee_SucessfulImpact.P_Aurora_Melee_SucessfulImpact"));
		if (MeleeSuccessImpact)
		{
			FTransform transform(FRotator(0), TargetCharacter->GetMesh()->GetSocketLocation("Impact"), FVector(1));
			SpawnRootedParticleAtLocation_Server(MeleeSuccessImpact, transform);
		}
	}
}

void AAuroraCharacter::CancelAbility()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_E))
	{
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_E);
		bSmoothMovement = false;
		bIsDashing = false;
	}
}

void AAuroraCharacter::Ability_E_Canceled()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	bSmoothMovement = false;
	bIsDashing = false;

	StopAllMontages_Server(0.2, true);
}

void AAuroraCharacter::Ability_LMB_Canceled()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	StopAllMontages_Server(0.2, true);
}

void AAuroraCharacter::Ability_RMB_Canceled()
{

}

void AAuroraCharacter::OnPreDamageReceived(float FinalDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("AAuroraCharacter::OnPreDamageReceived() FinalDamage: %f"), FinalDamage);
}


void AAuroraCharacter::ExecuteSomethingSpecial()
{
	UKismetSystemLibrary::PrintString(GetWorld(), TEXT("[AAuroraCharacter::ExecuteSomethingSpecial] ExecuteSomethingSpecial function called."), true, true, FLinearColor::Red, 2.0f, NAME_None);
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("[AAuroraCharacter::ExecuteSomethingSpecial] Before Movementspeed: %f."), StatComponent->GetMovementSpeed()), true, true, FLinearColor::Red, 2.0f, NAME_None);

	FDamageInformation DamageInformation;
	DamageInformation.AbilityID = EAbilityID::None;
	DamageInformation.CrowdControls.Add(FCrowdControlInformation(EBaseCrowdControl::Stun, 5.0f, 0.0f));

	ApplyDamage_Server(this, DamageInformation, GetController(), this);
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("[AAuroraCharacter::ExecuteSomethingSpecial] After Movementspeed: %f."), StatComponent->GetMovementSpeed()), true, true, FLinearColor::Red, 2.0f, NAME_None);
}

void AAuroraCharacter::Ability_LMB_AttackEnded()
{
	Ability_LMB_CurrentComboCount = 0;

	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->StopAllMontages(0.2f);
	StopAllMontages_Server(0.2, false);
}

void AAuroraCharacter::SetMovementSpeed_Server_Implementation(const float InMaxWalkSpeed, const float InMaxAcceleration)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->MaxWalkSpeed = 2000.f;
		MovementComponent->MaxAcceleration = 100000.f;
	}
}

void AAuroraCharacter::SpawnFreezeSegments_Server_Implementation(FTransform Transform, FCachedParticleInfo ParticleInfo, FDamageInformation DamageInformation)
{
	if (HasAuthority())
	{
		SpawnFreezeSegments_Multicast(Transform, ParticleInfo, DamageInformation);
	}
}

void AAuroraCharacter::SpawnFreezeSegments_Multicast_Implementation(FTransform Transform, FCachedParticleInfo ParticleInfo, FDamageInformation DamageInformation)
{
	if (HasAuthority())
	{
		return;
	}

	if (FreezeSegmentClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] AAuroraCharacter::SpawnCircularParticles - Freeze Segment Class is nullptr."));
		return;
	}

	AFreezeSegment* NewParticleActor = Cast<AFreezeSegment>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), FreezeSegmentClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (NewParticleActor != nullptr)
	{
		NewParticleActor->InitializeParticle(ParticleInfo.Radius, ParticleInfo.NumParicles, ParticleInfo.Lifetime, ParticleInfo.Rate, ParticleInfo.Scale, DamageInformation);
		UGameplayStatics::FinishSpawningActor(NewParticleActor, Transform);
	}
}

bool AAuroraCharacter::ValidateAbilityUsage()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Death))
	{
		return false;
	}

	if (bCtrlKeyPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_Q] Ability cannot be used because the Ctrl key is pressed."));
		return false;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(AbilityStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAuroraCharacter::Ability_Q] StatComponent, AbilityStatComponent, or AnimInstance is null."));
		return false;
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction))
	{
		return true;
	}

	return false;
}

void AAuroraCharacter::OnAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent)
{
	if (!HasAuthority())
	{
		return;
	}

	Super::OnAbilityUse(AbilityID, TriggerEvent);

	LogCharacterState(CharacterState, TEXT("AAuroraCharacter::OnAbilityUse Before CharacterState"));

	switch (AbilityID)
	{
	case EAbilityID::Ability_LMB:
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::Ability_LMB);
		break;
	case EAbilityID::Ability_Q:
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);
		EnumAddFlags(CharacterState, EBaseCharacterState::Ability_Q);
		break;
	case EAbilityID::Ability_E:
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);
		EnumAddFlags(CharacterState, EBaseCharacterState::Ability_E);
		break;
	case EAbilityID::Ability_R:
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);
		EnumAddFlags(CharacterState, EBaseCharacterState::Ability_R);
		break;
	case EAbilityID::Ability_RMB:
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(CharacterState, EBaseCharacterState::AbilityUsed);
		EnumAddFlags(CharacterState, EBaseCharacterState::Ability_RMB);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown AbilityID: %d"), static_cast<int32>(AbilityID));
		break;
	}

	LogCharacterState(CharacterState, TEXT("AAuroraCharacter::OnAbilityUse After CharacterState"));
}

void AAuroraCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MontageEnded called with null Montage"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MontageEnded: Montage '%s' ended. Interrupted: %s on %s"), *Montage->GetName(), bInterrupted ? TEXT("True") : TEXT("False"), HasAuthority() ? TEXT("server") : TEXT("client"));

	if (!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (Montage->GetName().Equals(TEXT("Ability_Q_Montage")))
		{
			ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Ability_Q);
		}
		else if (Montage->GetName().Equals(TEXT("Ability_E_Montage")))
		{
			bUseControllerRotationYaw = true;
			ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Ability_E);
		}
		else if (Montage->GetName().Equals(TEXT("Ability_R_Montage")))
		{
			ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Ability_R);
		}
		else if (Montage->GetName().Equals(TEXT("Ability_LMB_Montage")))
		{
			ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Ability_LMB);
		}
		else if (Montage->GetName().Equals(TEXT("Ability_RMB_Montage")))
		{
			ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Ability_RMB);
		}
	}

	LogCharacterState(CharacterState, TEXT("AAuroraCharacter::MontageEnded After CharacterState"));
}

void AAuroraCharacter::OnRep_CharacterStateChanged()
{
	Super::OnRep_CharacterStateChanged();

}

void AAuroraCharacter::OnRep_CrowdControlStateChanged()
{

}