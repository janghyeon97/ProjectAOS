// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SparrowCharacter.h"
#include "Components/StatComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/AbilityStatComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/InputConfigData.h"
#include "Animations/PlayerAnimInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Structs/DamageInfomationStruct.h"
#include "Props/ArrowBase.h"

ASparrowCharacter::ASparrowCharacter()
{
	static ConstructorHelpers::FClassFinder<UClass> ARROW_CLASS(TEXT("/Game/ProjectAOS/Characters/Sparrow/Blueprints/BP_Arrow.BP_Arrow"));
	if (ARROW_CLASS.Succeeded()) BasicArrowClass = ARROW_CLASS.Class;

	InitializeAbilityMontages();
	InitializeAbilityParticles();

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	AbilityStatComponent = CreateDefaultSubobject<UAbilityStatComponent>(TEXT("AbilityStatComponent"));

	if (HasAuthority())
	{
		StatComponent->SetIsReplicated(true);
		AbilityStatComponent->SetIsReplicated(true);
	}

	BowParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BowParticleSystem"));
	BowParticleSystem->SetAutoActivate(false);
	BowParticleSystem->SetupAttachment(GetMesh());
	BowParticleSystem->SetAutoAttachParams(GetMesh(), FName("BowEmitterSocket"), EAttachLocation::KeepRelativeOffset);

	Ability_Q_Range = 2200.f;

	Ability_R_Duration = 4.f;
	Ability_R_Angle = 10.f;
	Ability_R_ArrowSpeed = 6500.f;
	Ability_R_Range = 3000.f;

	Ability_LMB_ArrowSpeed = 8000.f;
	Ability_LMB_Range = 3000.f;

	Ability_RMB_ArrowSpeed = 2000.f;
	Ability_RMB_Range = 3000.f;

	SelectedCharacterIndex = 2;

	PrimaryActorTick.bCanEverTick = true;
}

void ASparrowCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_Q) && IsLocallyControlled())
	{
		FVector TempVector = GetImpactPoint(Ability_Q_Range);
		FVector TraceStartLocation = FVector(TempVector.X, TempVector.Y, TempVector.Z + 100.f);
		FVector TraceEndLocation = FVector(TempVector.X, TempVector.Y, GetActorLocation().Z - 100.f);

		FHitResult HitResult;
		FCollisionQueryParams params(NAME_None, false, this);
		bool bResult = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStartLocation,
			TraceEndLocation,
			ECollisionChannel::ECC_Visibility,
			params
		);

		if (bResult)
		{
			FVector DecalSpawnLocation = HitResult.Location;
			if (::IsValid(TargetDecalActor))
			{
				TargetDecalActor->Destroy();

				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Instigator = this;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

				TargetDecalActor = GetWorld()->SpawnActor<AActor>(TargetDecalClass, FTransform(FRotator(0), DecalSpawnLocation, FVector(1)), SpawnParameters);
			}
			else
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Instigator = this;
				SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

				TargetDecalActor = GetWorld()->SpawnActor<AActor>(TargetDecalClass, FTransform(FRotator(0), DecalSpawnLocation, FVector(1)), SpawnParameters);
			}
		}
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_RMB))
	{
		ChangeCameraLength(200.f);
	}
	else
	{
		ChangeCameraLength(500.f);
	}
}

void ASparrowCharacter::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance->OnMontageEnded.AddDynamic(this, &ASparrowCharacter::MontageEnded);

	GetMesh()->GetMaterialByName(FName("Arrow"));
}

void ASparrowCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
		{
			if (::IsValid(EnhancedInputComponent))
			{
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Triggered, this, &ASparrowCharacter::Move);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &ASparrowCharacter::Look);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &ASparrowCharacter::Look);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &ASparrowCharacter::Jump);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_Q_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_Q);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_Q_Action, ETriggerEvent::Triggered, this, &ASparrowCharacter::Ability_Q_Fire);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_E_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_E);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_R_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_R);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_LMB_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_LMB);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_RMB_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_RMB);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_RMB_Action, ETriggerEvent::Canceled, this, &ASparrowCharacter::Ability_RMB_Canceled);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_RMB_Action, ETriggerEvent::Triggered, this, &ASparrowCharacter::Ability_RMB_Fire);
			}
		}
	));
}

void ASparrowCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASparrowCharacter::InitializeAbilityMontages()
{
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_Q_MONTAGE(TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Q_Ability_Montage.Q_Ability_Montage"));
	if (ABILITY_Q_MONTAGE.Succeeded()) Ability_Q_Montage = ABILITY_Q_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_E_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_E_Montage.Ability_E_Montage"));
	if (ABILITY_E_MONTAGE.Succeeded()) Ability_E_Montage = ABILITY_E_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_R_MONTAGE(TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Ability_R_Montage.Ability_R_Montage"));
	if (ABILITY_R_MONTAGE.Succeeded()) Ability_R_Montage = ABILITY_R_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_LMB_MONTAGE(TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Med_Montage.Primary_Fire_Med_Montage"));
	if (ABILITY_LMB_MONTAGE.Succeeded()) Ability_LMB_Montage = ABILITY_LMB_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_LMB_FAST_MONTAGE(TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Fast_Montage.Primary_Fire_Fast_Montage"));
	if (ABILITY_LMB_FAST_MONTAGE.Succeeded()) Ability_LMB_FastMontage = ABILITY_LMB_FAST_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_LMB_SLOW_MONTAGE(TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Slow_Montage.Primary_Fire_Slow_Montage"));
	if (ABILITY_LMB_SLOW_MONTAGE.Succeeded()) Ability_LMB_SlowMontage = ABILITY_LMB_SLOW_MONTAGE.Object;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ABILITY_RMB_MONTAGE(TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
	if (ABILITY_RMB_MONTAGE.Succeeded()) Ability_RMB_Montage = ABILITY_RMB_MONTAGE.Object;
}

void ASparrowCharacter::InitializeAbilityParticles()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ABILITY_Q_RAINOFARROWS(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/RainOfArrows/FX/P_RainofArrows.P_RainofArrows"));
	if (ABILITY_Q_RAINOFARROWS.Succeeded()) Ability_Q_RainOfArrows = ABILITY_Q_RAINOFARROWS.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ARROW_PARTICLE_LMB(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_PrimaryAttack.P_Sparrow_PrimaryAttack"));
	if (ARROW_PARTICLE_LMB.Succeeded()) ArrowParticle_LMB = ARROW_PARTICLE_LMB.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ARROW_PARTICLE_RMB(TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/DrawABead/FX/P_Sparrow_RMB.P_Sparrow_RMB"));
	if (ARROW_PARTICLE_RMB.Succeeded()) ArrowParticle_RMB = ARROW_PARTICLE_RMB.Object;
}

void ASparrowCharacter::Move(const FInputActionValue& InValue)
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

void ASparrowCharacter::Look(const FInputActionValue& InValue)
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
	Ability_Q 함수는 캐릭터의 Q 스킬을 실행합니다.
	AbilityStatComponent가 null이거나 Ctrl 키가 눌려 있으면 스킬을 사용할 수 없습니다.

	- UniqueValue[0]: Duration		지속시간
	- UniqueValue[1]: Range			사거리

	1. AbilityStatComponent가 유효한지 확인합니다.
	2. Ctrl 키가 눌려 있는지 확인합니다.
	3. 스킬이 준비되었고, 캐릭터가 행동을 전환할 수 있는 상태인지 확인합니다.
	4. 조건이 충족되면 캐릭터 상태를 업데이트하고 스킬의 고유 값을 가져옵니다.
	5. 애니메이션 몽타주를 재생하고, 스킬의 히트 체크를 수행합니다.
	6. 조건이 충족되지 않으면 스킬이 준비되지 않았음을 알립니다.
*/
void ASparrowCharacter::Ability_Q()
{
	if (!ValidateAbilityUsage())
	{
		return;
	}



	bool bIsAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_Q);
	if (bIsAbilityReady)
	{
		ServerNotifyAbilityUse(EAbilityID::Ability_Q, ETriggerEvent::Started);

		const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_Q);
		Ability_Q_Range = AbilityStatTable.Range;

		if (::IsValid(Ability_Q_Montage))
		{
			AnimInstance->PlayMontage(Ability_Q_Montage, 1.0f);
			PlayMontage_Server(Ability_Q_Montage, 1.0f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q] Ability_Q_Montageis null."));
			return;
		}

		Ability_Q_CheckHit();
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}


/*
	Ability_Q_Fire 함수는 캐릭터의 Q 스킬을 발동하는 역할을 합니다.
	1. AbilityStatComponent가 유효한지 확인합니다. 유효하지 않으면 함수 실행을 종료하고 경고 로그를 출력합니다.
	2. 캐릭터 상태가 Ability_Q 인지 확인합니다.
	3. Ability_Q 상태라면, 애니메이션 몽타주의 현재 위치와 섹션 길이를 계산하여 남은 시간을 구합니다.
	4. 남은 시간 동안 타이머를 설정하고, 타이머가 만료되면 화살을 생성하고, 파티클을 생성하며, 타겟 데칼을 파괴하고, 애니메이션 섹션을 전환합니다.
	5. SpawnArrow_Server, SpawnRootedParticleAtLocation_Server 등의 서버 함수를 호출하여 필요한 오브젝트를 생성합니다.
*/

void ASparrowCharacter::Ability_Q_Fire()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_Q) == false)
	{
		return;
	}

	//AbilityStatComponent->UseAbility(EAbilityID::Ability_Q, GetWorld()->GetTimeSeconds());
	//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_Q);

	ServerNotifyAbilityUse(EAbilityID::Ability_Q, ETriggerEvent::Triggered);

	// 애니메이션 몽타주의 현재 위치와 섹션 길이를 계산
	if (::IsValid(Ability_Q_Montage) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Fire] Ability_Q_Montage is null."));
		return;
	}

	float CurrentPosition = AnimInstance->Montage_GetPosition(Ability_Q_Montage);
	float MontageLength = Ability_Q_Montage->GetSectionLength(0);
	float RemainingTime = MontageLength - CurrentPosition;

	int32 TimerID = static_cast<uint32>(EAbilityID::Ability_Q);
	auto TimerCallback = [this, TimerID]()
		{
			// 화살 생성 위치와 회전 계산
			FVector ArrowAnchorLocation = GetMesh()->GetSocketLocation(TEXT("arrow_anchor"));
			FVector ArrowLocation = GetMesh()->GetSocketLocation(TEXT("Arrow"));
			FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromX(ArrowAnchorLocation - ArrowLocation);
			FTransform SpawnTransform = FTransform(SpawnRotation, ArrowAnchorLocation, FVector(1));

			FArrowProperties ArrowProperties;
			ArrowProperties.Range = 1000.f;
			ArrowProperties.Speed = 5000.f;

			if (::IsValid(BasicArrowClass))
			{
				SpawnArrow_Server(BasicArrowClass, SpawnTransform, ArrowProperties, FDamageInfomation());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Fire] BasicArrowClassis null."));
			}

			SpawnRootedParticleAtLocation_Server(Ability_Q_RainOfArrows, FTransform(FRotator(0), TargetDecalActor->GetActorLocation(), FVector(1)));

			if (::IsValid(TargetDecalActor))
			{
				TargetDecalActor->Destroy();
			}

			FName NextSectionName = FName("Fire");
			if (::IsValid(Ability_Q_Montage))
			{
				AnimInstance->Montage_JumpToSection(NextSectionName, Ability_Q_Montage);
				MontageJumpToSection_Server(Ability_Q_Montage, NextSectionName, 1.0f);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Fire] Ability_Q_Montage is null."));
			}

			ClearGameTimer(TimerHandleList, TimerID);
		};


	SetGameTimer(TimerHandleList, TimerID, TimerCallback, 0.1, false, RemainingTime);
}

/*
	1. UniqueValue[0]: BonusAttackSpeed			추가 공격속도
	2. UniqueValue[1]: BonusMovementSpeed		추가 이동속도
*/
void ASparrowCharacter::Ability_E()
{

}


/*
	Ability_R 함수는 캐릭터의 R 스킬을 실행합니다.
	1. UniqueValue[0]: Duration				지속시간
	2. UniqueValue[1]: SideArrowsDamage		사이드 화살 데미지 퍼센트
	3. UniqueValue[2]: SideArrowsAngle		사이드 화살 각도
	4. UniqueValue[3]: ArrowSpeed			화살 속도
*/

void ASparrowCharacter::Ability_R()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_R);
	if (bAbilityReady)
	{
		// Ability를 사용하고 쿨다운을 시작하는 코드는 주석 처리되어 있음
		// AbilityStatComponent->UseAbility(EAbilityID::Ability_R, GetWorld()->GetTimeSeconds());
		// AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_R);

		ServerNotifyAbilityUse(EAbilityID::Ability_R, ETriggerEvent::None);

		const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_R);
		TMap<FString, float> UniqueAttributes = AbilityStatTable.GetUniqueAttributesMap();

		Ability_R_Duration = UniqueAttributes.Contains("Duration") ? UniqueAttributes["Duration"] : 0.f;
		Ability_R_SideDamage = UniqueAttributes.Contains("SideArrowsDamage") ? UniqueAttributes["SideArrowsDamage"] : 0.f;
		Ability_R_Angle = UniqueAttributes.Contains("SideArrowsAngle") ? UniqueAttributes["SideArrowsAngle"] : 0.f;
		Ability_R_ArrowSpeed = UniqueAttributes.Contains("ArrowSpeed") ? UniqueAttributes["ArrowSpeed"] : 0.f;
		Ability_R_Range = AbilityStatTable.Range;
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}



/**
 * LMB 능력을 실행합니다.
 * Ctrl 키가 눌려있거나 필요한 컴포넌트가 유효하지 않은 경우 능력을 사용할 수 없습니다.
 * 능력이 준비되고 캐릭터 상태가 SwitchAction을 포함하는 경우 능력을 실행합니다.
 * Ability_R 상태에서는 궁극기 화살을 발사하고, 그렇지 않으면 기본 화살을 발사합니다.
 *
 * [Ability LMB]
 * 1. UniqueValue[0]: Speed      화살 속도
 * 2. UniqueValue[1]: Range      화살 사거리
 */
void ASparrowCharacter::Ability_LMB()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_LMB);
	if (!bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	AbilityStatComponent->UseAbility(EAbilityID::Ability_LMB, GetWorld()->GetTimeSeconds());
	AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_LMB);
	ServerNotifyAbilityUse(EAbilityID::Ability_LMB, ETriggerEvent::None);

	Ability_LMB_AnimLength = 1.f;
	Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);
	Ability_LMB_ImpactPoint = GetImpactPoint();

	FVector ArrowSpawnLocation = GetMesh()->GetSocketLocation(FName("Arrow"));
	FRotator ArrowSpawnRotation = UKismetMathLibrary::MakeRotFromX(Ability_LMB_ImpactPoint - ArrowSpawnLocation);

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_R))
	{
		ExecuteAbilityR(ArrowSpawnLocation, ArrowSpawnRotation);
	}
	else
	{
		ExecuteAbilityLMB(ArrowSpawnLocation, ArrowSpawnRotation);
	}
}

void ASparrowCharacter::ExecuteAbilityR(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation)
{
	const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_R);

	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float Character_AbilityPower = StatComponent->GetAbilityPower();

	const float BaseAttackDamage = AbilityStatTable.AttackDamage;
	const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
	const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
	const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

	const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

	FArrowProperties ArrowProperties;
	ArrowProperties.PierceCount = 0;
	ArrowProperties.Speed = Ability_R_ArrowSpeed;
	ArrowProperties.Range = Ability_R_Range;

	FDamageInfomation DamageInfomation;
	DamageInfomation.AbilityID = EAbilityID::Ability_R;
	DamageInfomation.AddDamage(EDamageType::Physical, FinalDamage);
	EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::AbilityEffects);

	AnimInstance->PlayMontage(Ability_LMB_Montage, Ability_LMB_PlayRate);
	PlayMontage_Server(Ability_LMB_Montage, Ability_LMB_PlayRate);

	if (UltimateArrowClass)
	{
		// 중앙 화살
		SpawnArrow_Server(UltimateArrowClass, FTransform(ArrowSpawnRotation, ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInfomation);

		DamageInfomation.PhysicalDamage = DamageInfomation.PhysicalDamage * Ability_R_SideDamage;

		// 사이드 화살
		SpawnArrow_Server(UltimateArrowClass, FTransform(ArrowSpawnRotation + FRotator(0.f, -Ability_R_Angle, 0.f), ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInfomation);
		SpawnArrow_Server(UltimateArrowClass, FTransform(ArrowSpawnRotation + FRotator(0.f, Ability_R_Angle, 0.f), ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInfomation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::ExecuteAbilityR] Cannot spawn arrow because UltimateArrowClass is null."));
	}
}

void ASparrowCharacter::ExecuteAbilityLMB(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation)
{
	const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB);
	TMap<FString, float> UniqueAttributes = AbilityStatTable.GetUniqueAttributesMap();

	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float Character_AbilityPower = StatComponent->GetAbilityPower();

	const float BaseAttackDamage = AbilityStatTable.AttackDamage;
	const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
	const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
	const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

	const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

	Ability_LMB_ArrowSpeed = UniqueAttributes.Contains("ArrowSpeed") ? UniqueAttributes["ArrowSpeed"] : 0.f;
	Ability_LMB_Range = AbilityStatTable.Range;

	FArrowProperties ArrowProperties;
	ArrowProperties.PierceCount = 0;
	ArrowProperties.Speed = Ability_LMB_ArrowSpeed;
	ArrowProperties.Range = Ability_LMB_Range;

	FDamageInfomation DamageInfomation;
	DamageInfomation.AbilityID = EAbilityID::Ability_LMB;
	DamageInfomation.AddDamage(EDamageType::Physical, FinalDamage);
	EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::OnHit);
	EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::OnAttack);

	const float AttackSpeed = StatComponent->GetAttackSpeed();
	UAnimMontage* MontageToPlay = GetMontageBasedOnAttackSpeed(AttackSpeed);

	AnimInstance->PlayMontage(MontageToPlay, Ability_LMB_PlayRate);
	PlayMontage_Server(MontageToPlay, Ability_LMB_PlayRate);

	if (BasicArrowClass)
	{
		SpawnArrow_Server(BasicArrowClass, FTransform(ArrowSpawnRotation, ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInfomation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::ExecuteAbilityLMB] Cannot spawn arrow because BasicArrowClass is null."));
	}
}

UAnimMontage* ASparrowCharacter::GetMontageBasedOnAttackSpeed(float AttackSpeed)
{
	if (AttackSpeed < 1.f)
	{
		return Ability_LMB_SlowMontage;
	}
	else if (AttackSpeed > 1.f && AttackSpeed < 2.0f)
	{
		return Ability_LMB_Montage;
	}
	else
	{
		Ability_LMB_AnimLength = 0.6f;
		Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);
		return Ability_LMB_FastMontage;
	}
}


/*
	1. UniqueVale[0]: Range		화살 사거리
	2. UniqueVale[0]: Speed		화살 속도
*/
void ASparrowCharacter::Ability_RMB()
{
	bool bCanUseAbility = ValidateAbilityUsage();
	if (!bCanUseAbility)
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_RMB);

	if (bAbilityReady)
	{
		ServerNotifyAbilityUse(EAbilityID::Ability_RMB, ETriggerEvent::Started);

		if (::IsValid(Ability_RMB_Montage) == false)
		{
			return;
		}

		AnimInstance->PlayMontage(Ability_RMB_Montage, 1.0f);
		PlayMontage_Server(Ability_RMB_Montage, 1.0f);
	}
	else
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
	}
}

void ASparrowCharacter::Ability_RMB_Canceled()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_RMB) == false)
	{
		return;
	}

	ServerNotifyAbilityUse(EAbilityID::Ability_RMB, ETriggerEvent::Canceled);

	AnimInstance->StopAllMontages(0.25f);
	StopAllMontages_Server(0.25f);
}


void ASparrowCharacter::Ability_RMB_Fire()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Death))
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_RMB))
	{
		//AbilityStatComponent->UseAbility(EAbilityID::Ability_RMB, GetWorld()->GetTimeSeconds());
		//AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_RMB);

		ServerNotifyAbilityUse(EAbilityID::Ability_RMB, ETriggerEvent::Triggered);


		const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_RMB);
		TMap<FString, float> UniqueAttributes = AbilityStatTable.GetUniqueAttributesMap();

		if (AbilityStatTable.IsValid() == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_RMB_Fire] AbilityStatTable is null."));
			return;
		}

		FVector ImpactPoint = GetImpactPoint();
		FVector ArrowSpawnLocation = GetMesh()->GetSocketLocation(FName("arrow_anchor"));
		FRotator ArrowSpawnRotation = UKismetMathLibrary::MakeRotFromX(ImpactPoint - ArrowSpawnLocation);
		FTransform ArrowSpawnTransform(ArrowSpawnRotation, ArrowSpawnLocation, FVector(1));

		const float Character_AttackDamage = StatComponent->GetAttackDamage();
		const float Character_AbilityPower = StatComponent->GetAbilityPower();

		const float BaseAttackDamage = AbilityStatTable.AttackDamage;
		const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
		const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
		const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

		const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);


		Ability_RMB_ArrowSpeed = UniqueAttributes.Contains("ArrowSpeed") ? UniqueAttributes["ArrowSpeed"] : 0.f;
		Ability_RMB_Range = AbilityStatTable.Range;

		FArrowProperties ArrowProperties;
		ArrowProperties.PierceCount = 3;
		ArrowProperties.Pierce_DamageReduction = 0.1f;
		ArrowProperties.Speed = Ability_RMB_ArrowSpeed;
		ArrowProperties.Range = Ability_RMB_Range;

		FDamageInfomation DamageInfomation;
		DamageInfomation.AbilityID = EAbilityID::Ability_RMB;
		DamageInfomation.AddDamage(EDamageType::Physical, FinalDamage);
		EnumAddFlags(DamageInfomation.AttackEffect, EAttackEffect::AbilityEffects);

		FName NextSectionName = FName("Fire");

		if (::IsValid(Ability_RMB_Montage))
		{
			AnimInstance->Montage_JumpToSection(NextSectionName, Ability_RMB_Montage);
			MontageJumpToSection_Server(Ability_RMB_Montage, NextSectionName, 1.0f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_RMB_Fire] Ability_RMB_Montage is null."));
		}

		if (::IsValid(PiercingArrowClass))
		{
			SpawnArrow_Server(PiercingArrowClass, ArrowSpawnTransform, ArrowProperties, DamageInfomation);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_RMB_Fire] Cannot spawn arrow because PiercingArrowClass is null."));
		}

	}
}

void ASparrowCharacter::Ability_Q_CheckHit()
{
}

void ASparrowCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == Ability_LMB_Montage)
	{
		EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
	}
	else
	{
		EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
		EnumRemoveFlags(CharacterState, EBaseCharacterState::AbilityUsed);
	}
}

/**
 * 카메라의 전방 벡터와 추적 범위에 따라 충돌 지점을 계산합니다.
 * 이 함수는 카메라 위치에서 추적 범위로 정의된 지점까지 라인을 추적합니다.
 * 만약 추적이 물체와 충돌하면, 충돌 지점으로 충돌 위치를 업데이트합니다.
 *
 * @param TraceRange 추적이 충돌을 확인해야 하는 최대 거리입니다.
 * @return 계산된 충돌 지점입니다.
 */
FVector ASparrowCharacter::GetImpactPoint(float TraceRange)
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	if (IsValid(CameraManager) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraManageris null."));
		return FVector::ZeroVector;
	}

	CrosshairLocation = CameraManager->GetCameraLocation();
	FVector ImpactPoint = CrosshairLocation + CameraManager->GetActorForwardVector() * TraceRange;

	FHitResult HitResult;
	FCollisionQueryParams params(NAME_None, false, this);
	bool bResult = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CrosshairLocation,
		ImpactPoint,
		ECollisionChannel::ECC_Visibility,
		params
	);

	if (bResult)
	{
		ImpactPoint = HitResult.Location;
		return ImpactPoint;
	}

	return ImpactPoint;
}


void ASparrowCharacter::ChangeCameraLength(float TargetLength)
{
	float CurrentArmLength = SpringArmComponent->TargetArmLength;

	//InterpolationAlpha = FMath::Clamp(RotationTimer / AnimLength, 0.f, 1.f);
	SpringArmComponent->TargetArmLength = FMath::InterpEaseInOut(CurrentArmLength, TargetLength, GetWorld()->GetDeltaSeconds(), 0.8);
}

// DestroyAfterSeconds 가 0 이상일 경우 DestroyAfterSeconds 만큼 딜레이 후 스폰된 화살을 파괴합니다.
void ASparrowCharacter::SpawnArrow_Server_Implementation(UClass* SpawnArrowClass, FTransform SpawnTransform, FArrowProperties InArrowProperties, FDamageInfomation InDamageInfomation)
{
	if (SpawnArrowClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ASparrowCharacter::SpawnArrow_Server - Spawn arrow class is nullptr."));
		return;
	}

	AArrowBase* NewArrowActor = Cast<AArrowBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), SpawnArrowClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (NewArrowActor != nullptr)
	{
		NewArrowActor->InitializeArrowActor(InArrowProperties, InDamageInfomation);
		UGameplayStatics::FinishSpawningActor(NewArrowActor, SpawnTransform);
	}
}

bool ASparrowCharacter::ValidateAbilityUsage()
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

	return true;
}

void ASparrowCharacter::ServerNotifyAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent)
{
	Super::ServerNotifyAbilityUse(AbilityID, TriggerEvent);

	int32 TimerID = 0;

	if (AbilityID == EAbilityID::Ability_Q)
	{
		if (EnumHasAnyFlags(TriggerEvent, ETriggerEvent::Started))
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
			EnumAddFlags(CharacterState, EBaseCharacterState::Ability_Q);
			UE_LOG(LogTemp, Log, TEXT("ServerNotifyAbilityUse: Ability_Q started. Updated CharacterState."));
		}
		else if (EnumHasAnyFlags(TriggerEvent, ETriggerEvent::Triggered))
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_Q);
			UE_LOG(LogTemp, Log, TEXT("ServerNotifyAbilityUse: Ability_Q triggered. Updated CharacterState."));
		}
	}
	else if (AbilityID == EAbilityID::Ability_E)
	{
		
	}
	else if (AbilityID == EAbilityID::Ability_R)
	{
		CharacterState = CharacterState | EBaseCharacterState::Ability_R;
		UE_LOG(LogTemp, Log, TEXT("ServerNotifyAbilityUse: Ability_R started. Updated CharacterState."));

		TimerID = static_cast<int32>(EAbilityID::Ability_R);
		SetGameTimer(AbilityTimer, TimerID,
			[this, TimerID]()
			{
				EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_R);
				ClearGameTimer(AbilityTimer, TimerID);
				UE_LOG(LogTemp, Log, TEXT("ServerNotifyAbilityUse: Ability_R timer ended. Updated CharacterState."));
			},
			0.1f, false, Ability_R_Duration);
	}
	else if (AbilityID == EAbilityID::Ability_RMB)
	{
		if (TriggerEvent == ETriggerEvent::Started)
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
			EnumAddFlags(CharacterState, EBaseCharacterState::Ability_RMB);
			UE_LOG(LogTemp, Log, TEXT("ServerNotifyAbilityUse: Ability_RMB started. Updated CharacterState."));
		}
		else
		{
			EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
			EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_RMB);
			UE_LOG(LogTemp, Log, TEXT("ServerNotifyAbilityUse: Ability_RMB stopped. Updated CharacterState."));
		}
	}
	else if (AbilityID == EAbilityID::Ability_LMB)
	{
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		UE_LOG(LogTemp, Log, TEXT("ServerNotifyAbilityUse: Ability_LMB triggered. Updated CharacterState."));
	}

	OnRep_CharacterStateChanged();
}



void ASparrowCharacter::OnRep_CharacterStateChanged()
{
	Super::OnRep_CharacterStateChanged();

	UE_LOG(LogTemp, Log, TEXT("[ASparrowCharacter::OnRep_CharacterStateChanged] OnRep_CharacterStateChanged"));

	if (::IsValid(BowParticleSystem))
	{
		EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_R) ? BowParticleSystem->Activate() : BowParticleSystem->Deactivate();
	}
}

