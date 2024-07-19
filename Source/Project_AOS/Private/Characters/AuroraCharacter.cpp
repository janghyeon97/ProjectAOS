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
#include "Props/FreezeSegment.h"


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

	Ability_RMB_QuadraticScale = 3.f;
	Ability_RMB_JumpScale = 60.f;

	SplinePointIndex = 0;

	bIsTumbling = false;
	bIsDashing = false;

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

	if (::IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AAuroraCharacter::MontageEnded);
		AnimInstance->OnStopBasicAttackNotifyBegin.BindUObject(this, &AAuroraCharacter::Ability_LMB_AttackEnded);
		//AnimInstance->OnAuroraDashEnded.BindUObject(this, &AAuroraCharacter::Ability_E_Ended);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::BeginPlay] AnimInstance is no vaild."));
	}

	//RegisterAbilityStage(EAbilityID::Ability_Q, 1, FAbilityStageFunction::CreateLambda([this]() { PlayeAbilityMontage(Ability_Q_Montage, 1.0); }));
	//RegisterAbilityStage(EAbilityID::Ability_Q, 2, FAbilityStageFunction::CreateLambda([this]() { Ability_Q_CalculateAndRequestParticleSpawn(); }));
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
			//EnumRemoveFlags(CharacterState, EBaseCharacterState::AbilityUsed);
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
	3. UniqueValue[2]: ParticleScale		링 파티클 크기
	4. UniqueValue[3]: NumParticles			링 파티클 개수
	5. UniqueValue[4]: FirstDelay			링 파티클 초기 지연시간
	6. UniqueValue[5]: Rate					링 파티클 생성 속도
*/
void AAuroraCharacter::Ability_Q()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_Q);
	if (!bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	// 능력 쿨타임 설정
	//AbilityStatComponent->UseAbility(EAbilityID::Ability_Q, GetWorld()->GetTimeSeconds());
	//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_Q);

	ServerNotifyAbilityUse(EAbilityID::Ability_Q, ETriggerEvent::None);
	LogCharacterState(CharacterState, TEXT("AAuroraCharacter::Ability_Q"));
	SaveCharacterTransform();

	// 능력 통계 값을 가져와서 고유 값을 확인
	const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_Q);;
	TMap<FString, float> UniqueAttributes = AbilityStatTable.GetUniqueAttributesMap();

	Ability_Q_Radius = AbilityStatTable.Radius != 0 ? AbilityStatTable.Radius : 440.f;
	Ability_Q_RingDuration = UniqueAttributes.Contains("RingDuration") ? UniqueAttributes["RingDuration"] : 2.f;
	Ability_Q_ParticleScale = UniqueAttributes.Contains("ParticleScale") ? UniqueAttributes["ParticleScale"] : 1.f;
	Ability_Q_NumParicles = UniqueAttributes.Contains("NumParticles") ? UniqueAttributes["NumParticles"] : 28;
	Ability_Q_FirstDelay = UniqueAttributes.Contains("FirstDelay") ? UniqueAttributes["FirstDelay"] : 0.35f;
	Ability_Q_Rate = UniqueAttributes.Contains("Rate") ? UniqueAttributes["Rate"] : 0.01f;

	FCachedParticleInfo ParticleInfomation;
	ParticleInfomation.Lifetime = Ability_Q_RingDuration;
	ParticleInfomation.NumParicles = Ability_Q_NumParicles;
	ParticleInfomation.Scale = Ability_Q_ParticleScale;
	ParticleInfomation.Rate = Ability_Q_Rate;
	ParticleInfomation.Radius = Ability_Q_Radius;

	// 데미지 계산
	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float Character_AbilityPower = StatComponent->GetAbilityPower();

	const float BaseAttackDamage = AbilityStatTable.AttackDamage ? AbilityStatTable.AttackDamage : 0;
	const float BaseAbilityPower = AbilityStatTable.AbilityDamage ? AbilityStatTable.AbilityDamage : 0;
	const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling ? AbilityStatTable.AD_PowerScaling : 0;
	const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling ? AbilityStatTable.AP_PowerScaling : 0;

	const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

	FDamageInfomation DamageInfomation;
	DamageInfomation.AbilityID = EAbilityID::Ability_Q;
	DamageInfomation.AttackEffect = EAttackEffect::AbilityEffects;
	DamageInfomation.CrowdControls.Add(FCrowdControlInformation(EBaseCrowdControl::Snare, 1.0));
	DamageInfomation.AddDamage(EDamageType::Magic, FinalDamage);

	FTransform Transform(UKismetMathLibrary::MakeRotFromX(LastForwardVector), LastCharacterLocation, FVector(1));

	PlayeAbilityMontage(Ability_Q_Montage, 1.0);
	SpawnFreezeSegments_Server(Transform, ParticleInfomation, DamageInfomation);
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

	if (::IsValid(ShieldTop) && ::IsValid(ShieldMiddle) && ::IsValid(ShieldBottom))
	{
		SpawnAttachedMeshAtLocation_Server(ShieldBottom, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, -50.0f), Ability_E_ShieldDuration);
		SpawnAttachedMeshAtLocation_Server(ShieldMiddle, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, 0.0f), Ability_E_ShieldDuration);
		SpawnAttachedMeshAtLocation_Server(ShieldTop, LastCharacterLocation * 10 + FVector(0.0f, 0.0f, 50.0f), Ability_E_ShieldDuration);
	}

	Ability_E_Server(TargetLocation);
	PlayeAbilityMontage(Ability_E_Montage, 1.0);
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
		FVector NewLocation = FMath::Lerp(LastCharacterLocation, Ability_E_TargetLocation, InterpolationAlpha) + FVector(0, 0, 60.f);

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
*/
void AAuroraCharacter::Ability_R()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
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

	ServerNotifyAbilityUse(EAbilityID::Ability_R, ETriggerEvent::None);
	SaveCharacterTransform();

	PlayeAbilityMontage(Ability_R_Montage, 1.0);

	Ability_R_Started_Server();

	if (::IsValid(UltimateWarmUp))
	{
		FTransform ParticleTransform(FRotator::ZeroRotator, LastCharacterLocation, FVector(1.0f));
		SpawnAttachedParticleAtLocation_Server(UltimateWarmUp, ParticleTransform);
	}
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

/**
 * @brief LMB 능력을 활성화합니다. Ctrl 키가 눌리거나 AbilityStatComponent가 없는 경우 능력을 사용할 수 없습니다.
 *        능력이 준비된 경우, 능력을 사용하고 쿨다운을 시작합니다. 첫 번째 공격과 후속 공격을 처리합니다.
 */
void AAuroraCharacter::Ability_LMB()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_LMB);
	if (bAbilityReady)
	{
		AbilityStatComponent->UseAbility(EAbilityID::Ability_LMB, GetWorld()->GetTimeSeconds());
		AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_LMB);

		ServerNotifyAbilityUse(EAbilityID::Ability_LMB, ETriggerEvent::None);

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
 * 캐릭터의 현재 위치와 이동 방향을 기반으로 목표 위치를 설정합니다.
 * 목표 위치로의 이동 중 장애물 충돌 여부를 확인하고, 충돌한 경우 적절히 처리합니다.
 *
 *	- UniqueValue[0]: Range			사거리
 */
void AAuroraCharacter::Ability_RMB()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	bool bIsAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_RMB);

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) && bIsAbilityReady)
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

		if (::IsValid(Ability_RMB_Montage))
		{
			int32 DirectionIndex = CalculateDirectionIndex();
			FName MontageSectionName = FName(*FString::Printf(TEXT("RMB%d"), DirectionIndex));

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

void AAuroraCharacter::Ability_Q_CheckHit()
{

}

void AAuroraCharacter::Ability_E_CheckHit()
{

}


/*
 *  
 * 
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
	// 폭발 파티클 생성
	if (UltimateExplode)
	{
		SpawnRootedParticleAtLocation_Server(UltimateExplode, FTransform(FRotator(0), GetActorLocation(), FVector(1)));
	}

	// 능력 스탯 테이블 및 고유 속성 가져오기
	const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_R);
	TMap<FString, float> UniqueAttributes = AbilityStatTable.GetUniqueAttributesMap();

	float CollisionSphereSize = AbilityStatTable.Radius > 0 ? AbilityStatTable.Radius : 850.f;
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
			const float SlowDuration = UniqueAttributes.Contains("SlowDuration") ? UniqueAttributes["SlowDuration"] : 1.5f;
			const float MovementSpeedSlow = UniqueAttributes.Contains("MovementSpeedSlow") ? UniqueAttributes["MovementSpeedSlow"] : 20.0f;

			FDamageInfomation DamageInfomation;
			DamageInfomation.AbilityID = EAbilityID::Ability_R;
			DamageInfomation.CrowdControls.Add(FCrowdControlInformation(EBaseCrowdControl::Slow, SlowDuration, MovementSpeedSlow));
			DamageInfomation.AddDamage(EDamageType::Magic, 0);

			ApplyDamage_Server(Character, DamageInfomation, GetController(), this);

			FrozenEnemies.Add(Character);
		}
	}

	const float ExplodeTime = UniqueAttributes.Contains("ExplodeTime") ? UniqueAttributes["ExplodeTime"] : 2.0f;
	const int32 TimerID = static_cast<uint32>(EAbilityID::Ability_R);

	// 폭발 타이머 설정
	auto TimerCallback = [this, TimerID, AbilityStatTable, UniqueAttributes, FrozenEnemies]()
		{
			// 데미지 계산
			const float Character_AttackDamage = StatComponent->GetAttackDamage();
			const float Character_AbilityPower = StatComponent->GetAbilityPower();

			const float BaseAttackDamage = AbilityStatTable.AttackDamage;
			const float BaseAbilityDamage = AbilityStatTable.AbilityDamage;
			const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
			const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

			const float HeroShatterAbilityDamage = UniqueAttributes.Contains("HeroShatterAbilityDamage") ? UniqueAttributes["HeroShatterAbilityDamage"] : 0.f;
			const float NonHeroShatterAbilityDamage = UniqueAttributes.Contains("NonHeroShatterAbilityDamage") ? UniqueAttributes["NonHeroShatterAbilityDamage"] : 0.f;
			const float InitialPowerScaling = UniqueAttributes.Contains("InitialPowerScaling") ? UniqueAttributes["InitialPowerScaling"] : 0.f;
			const float PowerScalingOnHero = UniqueAttributes.Contains("PowerScalingOnHero") ? UniqueAttributes["PowerScalingOnHero"] : 0.f;
			const float PowerScalingOnNonHero = UniqueAttributes.Contains("PowerScalingOnNonHero") ? UniqueAttributes["PowerScalingOnNonHero"] : 0.f;
			const float ChainRadius = UniqueAttributes.Contains("ChainRadius") ? UniqueAttributes["ChainRadius"] : 500.f;
			const float StunDuration = UniqueAttributes.Contains("StunDuration") ? UniqueAttributes["StunDuration"] : 1.f;

			float FinalDamage = 0;

			for (auto& FrozenEnemy : FrozenEnemies)
			{
				if (!::IsValid(FrozenEnemy)) continue;

				FinalDamage = BaseAbilityDamage + Character_AbilityPower * InitialPowerScaling;

				FDamageInfomation DamageInfomation;
				DamageInfomation.AbilityID = EAbilityID::Ability_R;
				DamageInfomation.AddDamage(EDamageType::Magic, FinalDamage);
				DamageInfomation.CrowdControls.Add(FCrowdControlInformation(EBaseCrowdControl::Stun, StunDuration));

				ApplyDamage_Server(FrozenEnemy, DamageInfomation, GetController(), this);

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

						FDamageInfomation ChainDamageInfomation;
						ChainDamageInfomation.AbilityID = EAbilityID::Ability_R;
						ChainDamageInfomation.AddDamage(EDamageType::Magic, FinalDamage);

						ApplyDamage_Server(FrozenEnemy, DamageInfomation, GetController(), this);
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
					const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB);

					const float Character_AttackDamage = StatComponent->GetAttackDamage();
					const float Character_AbilityPower = StatComponent->GetAbilityPower();

					const float BaseAttackDamage = AbilityStatTable.AttackDamage;
					const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
					const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
					const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

					const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

					FDamageInfomation DamageInfomation;
					DamageInfomation.AbilityID = EAbilityID::Ability_LMB;
					EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::OnHit);
					EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::OnAttack);
					DamageInfomation.AddDamage(EDamageType::Physical, FinalDamage);

					ApplyDamage_Server(Character, DamageInfomation, GetController(), this);

					/* Spawn HitSuccessImpact */
					FTransform transform(FRotator(0), Character->GetMesh()->GetSocketLocation("Impact"), FVector(1));
					SpawnRootedParticleAtLocation_Server(MeleeSuccessImpact, transform);
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
	GetCrowdControl(EBaseCrowdControl::Blind, 2.0);
}

void AAuroraCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == Ability_LMB_Montage)
	{

	}
	else if (Montage == Ability_E_Montage)
	{
		bUseControllerRotationYaw = true;
	}
	else
	{
		/*ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::Move);
		ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::Jump);
		ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::SwitchAction);
		ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::AbilityUsed);*/
	}
}

void AAuroraCharacter::Ability_LMB_AttackEnded()
{
	Ability_LMB_CurrentComboCount = 0;

	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->StopAllMontages(0.2f);
	StopAllMontages_Server(0.2);
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

void AAuroraCharacter::SpawnFreezeSegments_Server_Implementation(FTransform Transform, FCachedParticleInfo ParticleInfo, FDamageInfomation DamageInfomation)
{
	if (HasAuthority())
	{
		SpawnFreezeSegments_Multicast(Transform, ParticleInfo, DamageInfomation);
	}
}

void AAuroraCharacter::SpawnFreezeSegments_Multicast_Implementation(FTransform Transform, FCachedParticleInfo ParticleInfo, FDamageInfomation DamageInfomation)
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
		NewParticleActor->InitializeParticle(ParticleInfo.Radius, ParticleInfo.NumParicles, ParticleInfo.Lifetime, ParticleInfo.Rate, ParticleInfo.Scale, DamageInfomation);
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

void AAuroraCharacter::PlayeAbilityMontage(UAnimMontage* Montage, float PlayRate)
{
	if (::IsValid(Montage) == false)
	{
		return;
	}

	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->PlayMontage(Montage, PlayRate);
	PlayMontage_Server(Montage, PlayRate);
}

void AAuroraCharacter::ServerNotifyAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent)
{
	Super::ServerNotifyAbilityUse(AbilityID, TriggerEvent);

	EBaseCharacterState NewState = CharacterState;

	LogCharacterState(CharacterState, TEXT("AAuroraCharacter::ServerNotifyAbilityUse Before CharacterState"));

	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
	case EAbilityID::Ability_E:
	case EAbilityID::Ability_R:
	case EAbilityID::Ability_RMB:
		EnumRemoveFlags(NewState, EBaseCharacterState::Move);
		EnumRemoveFlags(NewState, EBaseCharacterState::SwitchAction);
		EnumAddFlags(NewState, EBaseCharacterState::AbilityUsed);
		break;
	case EAbilityID::Ability_LMB:
		EnumRemoveFlags(NewState, EBaseCharacterState::SwitchAction);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown AbilityID: %d"), static_cast<int32>(AbilityID));
		break;
	}

	CharacterState = NewState;
	SetCharacterState(static_cast<uint32>(NewState));

	UpdateCharacterState(ReplicatedCharacterState);
	LogCharacterState(CharacterState, TEXT("AAuroraCharacter::ServerNotifyAbilityUse After CharacterState"));
}

void AAuroraCharacter::OnRep_CharacterStateChanged()
{
	Super::OnRep_CharacterStateChanged();

}