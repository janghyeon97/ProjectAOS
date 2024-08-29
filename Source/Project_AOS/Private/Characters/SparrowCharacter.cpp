// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SparrowCharacter.h"
#include "Components/StatComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Game/AOSGameInstance.h"
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
#include "Structs/CustomCombatData.h"
#include "Props/ArrowBase.h"

ASparrowCharacter::ASparrowCharacter()
{
	static ConstructorHelpers::FClassFinder<UClass> ARROW_CLASS(TEXT("/Game/ProjectAOS/Characters/Sparrow/Blueprints/BP_Arrow.BP_Arrow"));
	if (ARROW_CLASS.Succeeded()) BasicArrowClass = ARROW_CLASS.Class;

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

	SelectedCharacterIndex = 2;
	ChampionName = "Sparrow";

	PrimaryActorTick.bCanEverTick = true;

	InitializeCharacterResources();
}


void ASparrowCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_Q) && IsLocallyControlled())
	{
		FImpactResult ImpactResult = GetImpactPoint(Ability_Q_Range);
		FVector ImpactPoint = ImpactResult.ImpactPoint;
		FVector TraceStart = FVector(ImpactPoint.X, ImpactPoint.Y, ImpactPoint.Z + 100.f);
		FVector TraceEnd = FVector(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z - 100.f);

		FHitResult GroundHitResult;
		FCollisionQueryParams CollisionParams(NAME_None, false, this);
		bool bGroundHit = GetWorld()->LineTraceSingleByChannel(
			GroundHitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility,
			CollisionParams
		);

		if (bGroundHit)
		{
			Ability_Q_DecalLocation = GroundHitResult.Location;

			if (::IsValid(TargetDecalActor))
			{
				TargetDecalActor->Destroy();
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

			TargetDecalActor = GetWorld()->SpawnActor<AActor>(TargetDecalClass, FTransform(FRotator(0), Ability_Q_DecalLocation, FVector(1)), SpawnParams);
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
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_Q_Action, ETriggerEvent::Canceled, this, &ASparrowCharacter::Ability_Q_Canceled);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_Q_Action, ETriggerEvent::Triggered, this, &ASparrowCharacter::Ability_Q_Fire);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_E_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_E);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_R_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_R);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_LMB_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_LMB);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_RMB_Action, ETriggerEvent::Started, this, &ASparrowCharacter::Ability_RMB);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_RMB_Action, ETriggerEvent::Canceled, this, &ASparrowCharacter::Ability_RMB_Canceled);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Ability_RMB_Action, ETriggerEvent::Triggered, this, &ASparrowCharacter::Ability_RMB_Fire);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->CallAFunctionAction, ETriggerEvent::Started, this, &ASparrowCharacter::ExecuteSomethingSpecial);
			}
		}
	));
}

void ASparrowCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASparrowCharacter::Move(const FInputActionValue& InValue)
{
	if (!EnumHasAnyFlags(CharacterState, EBaseCharacterState::Move) || EnumHasAnyFlags(CrowdControlState, EBaseCrowdControl::Stun) || EnumHasAnyFlags(CrowdControlState, EBaseCrowdControl::Snare))
	{
		return;
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
	UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q] Ability_Q Called."));

	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bIsAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_Q);
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) == false || !bIsAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	ServerNotifyAbilityUse(EAbilityID::Ability_Q, ETriggerEvent::Started);

	Ability_Q_Range = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_Q).Range;
	PlayMontage("Q", 1.0, NAME_None, TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_Q_Montage.Ability_Q_Montage"));
}

void ASparrowCharacter::Ability_Q_Canceled()
{
	UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Canceled] Ability_Q_Canceled Called."));

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_Q) == false)
	{
		return;
	}

	ServerNotifyAbilityUse(EAbilityID::Ability_Q, ETriggerEvent::Canceled);

	if (::IsValid(TargetDecalActor)) TargetDecalActor->Destroy();
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
	UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Fire] Ability_Q_Fire Called."));

	if (!ValidateAbilityUsage())
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
	UAnimMontage* Montage = GetOrLoadMontage("Q", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_Q_Montage.Ability_Q_Montage"));

	float RemainingTime = 0;
	if (AnimInstance->Montage_IsActive(Montage))
	{
		float CurrentPosition = AnimInstance->Montage_GetPosition(Montage);
		float MontageLength = Montage->GetSectionLength(0);
		RemainingTime = MontageLength - CurrentPosition;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Fire] RemainingTime %f."), RemainingTime);

	int32 TimerID = static_cast<uint32>(EAbilityID::Ability_Q);
	auto TimerCallback = [this, TimerID, Montage]()
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
				SpawnArrow_Server(BasicArrowClass, nullptr, SpawnTransform, ArrowProperties, FDamageInformation());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Fire] BasicArrowClassis null."));
			}

			UParticleSystem* RainOfArrows = GetOrLoadParticle("RainOfArrows", TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/RainOfArrows/FX/P_RainofArrows.P_RainofArrows"));
			SpawnRootedParticleAtLocation_Server(RainOfArrows, FTransform(FRotator(0), TargetDecalActor->GetActorLocation(), FVector(1)));

			if (::IsValid(TargetDecalActor))
			{
				TargetDecalActor->Destroy();
			}

			FName NextSectionName = FName("Fire");
			if (Montage)
			{
				AnimInstance->Montage_JumpToSection(NextSectionName, Montage);
				MontageJumpToSection_Server(Montage, NextSectionName);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_Q_Fire] Ability_Q_Montage is null."));
			}

			ClearGameTimer(AbilityTimer, TimerID);
		};


	SetGameTimer(AbilityTimer, TimerID, TimerCallback, RemainingTime <= 0 ? 0.0001 : RemainingTime, false, RemainingTime);
}

/*
	1. UniqueValue[0]: BonusAttackSpeed			추가 공격속도
	2. UniqueValue[1]: BonusMovementSpeed		추가 이동속도
*/
void ASparrowCharacter::Ability_E()
{
	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_E);
	if (!bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	ServerNotifyAbilityUse(EAbilityID::Ability_E, ETriggerEvent::None);
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
	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_R);
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) == false || !bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	// Ability를 사용하고 쿨다운을 시작하는 코드는 주석 처리되어 있음
	// AbilityStatComponent->UseAbility(EAbilityID::Ability_R, GetWorld()->GetTimeSeconds());
	// AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_R);

	ServerNotifyAbilityUse(EAbilityID::Ability_R, ETriggerEvent::None);
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
	// 능력 사용 가능 여부를 검증
	if (!ValidateAbilityUsage())
	{
		return;
	}

	// 능력이 준비되었는지 확인
	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_LMB);
	if (!EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) || !bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	// 능력 사용 및 쿨다운 시작
	AbilityStatComponent->UseAbility(EAbilityID::Ability_LMB, GetWorld()->GetTimeSeconds());
	AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_LMB);
	ServerNotifyAbilityUse(EAbilityID::Ability_LMB, ETriggerEvent::None);

	// 애니메이션 재생 속도 설정
	Ability_LMB_AnimLength = 1.f;
	Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);

	// 능력 범위 설정
	const float Range = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB).Range;

	// 충돌 지점 계산
	FImpactResult ImpactResult = GetSweepImpactPoint(Range > 0 ? Range : 10000.f);

	Ability_LMB_ImpactPoint = ImpactResult.ImpactPoint;

	if (ImpactResult.bHit && ::IsValid(ImpactResult.HitResult.GetActor()))
	{
		// 충돌한 물체의 충돌 채널을 확인
		ECollisionChannel HitChannel = ImpactResult.HitResult.GetComponent()->GetCollisionObjectType();
		if (HitChannel == ECC_WorldStatic || HitChannel == ECC_WorldDynamic)
		{
			// ECC_WorldStatic 또는 ECC_WorldDynamic에 대한 처리
		}
		else
		{
			ProcessImpactWithNonStaticActor(ImpactResult);
		}
	}

	// 화살의 스폰 위치 및 회전 계산
	FVector ArrowSpawnLocation = GetMesh()->GetSocketLocation(FName("Arrow"));
	FRotator ArrowSpawnRotation = UKismetMathLibrary::MakeRotFromX(Ability_LMB_ImpactPoint - ArrowSpawnLocation);
	ArrowSpawnRotation.Normalize();

	// 디버그 라인 그리기
	DrawDebugLine(GetWorld(), ArrowSpawnLocation, Ability_LMB_ImpactPoint, FColor::Green, false, 5.0f, 0, 1.0f);

	// 궁극기 상태인지 확인하여 능력 실행
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_R))
	{
		ExecuteAbilityR(ArrowSpawnLocation, ArrowSpawnRotation);
	}
	else
	{
		ExecuteAbilityLMB(ArrowSpawnLocation, ArrowSpawnRotation);
	}
}

void ASparrowCharacter::ProcessImpactWithNonStaticActor(const FImpactResult& ImpactResult)
{
	AActor* HitActor = ImpactResult.HitResult.GetActor();
	if (HitActor)
	{
		// Skeletal Mesh 컴포넌트 확인
		USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(HitActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
		if (SkeletalMeshComponent)
		{
			FindAndSetClosestBone(ImpactResult, SkeletalMeshComponent);
		}
		else
		{
			// Skeletal Mesh 컴포넌트가 없을 경우, 충돌 지점을 설정
			Ability_LMB_ImpactPoint = ImpactResult.ImpactPoint;
			UE_LOG(LogTemp, Warning, TEXT("No Skeletal Mesh Component found on Actor: %s"), *HitActor->GetName());
		}
	}
}

void ASparrowCharacter::FindAndSetClosestBone(const FImpactResult& ImpactResult, USkeletalMeshComponent* SkeletalMeshComponent)
{
	FVector ImpactPoint = ImpactResult.HitResult.ImpactPoint;
	FString ClosestBoneName;
	float ClosestDistanceSq = FLT_MAX;

	// 모든 Bone의 위치를 확인하여 가장 가까운 Bone을 찾음
	for (int32 i = 0; i < SkeletalMeshComponent->GetNumBones(); i++)
	{
		FName BoneName = SkeletalMeshComponent->GetBoneName(i);
		FVector BoneLocation = SkeletalMeshComponent->GetBoneLocation(BoneName);

		float DistanceSq = FVector::DistSquared(BoneLocation, ImpactPoint);

		// 가장 가까운 Bone을 업데이트
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestBoneName = BoneName.ToString();
		}
	}

	// 가장 가까운 Bone의 위치를 Ability_LMB_ImpactPoint에 저장
	if (!ClosestBoneName.IsEmpty())
	{
		Ability_LMB_ImpactPoint = SkeletalMeshComponent->GetBoneLocation(FName(*ClosestBoneName));

		// 최종적으로 선택된 Bone 이름과 위치를 로그로 출력
		UE_LOG(LogTemp, Log, TEXT("Selected Closest Bone: %s, Location: %s"), *ClosestBoneName, *Ability_LMB_ImpactPoint.ToString());
	}
}


void ASparrowCharacter::ExecuteAbilityR(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation)
{
	const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_R);
	TMap<FString, float> UniqueAttributes = AbilityStatTable.GetUniqueAttributesMap();

	const float Ability_R_Duration = GetUniqueAttribute(EAbilityID::Ability_R, "Duration", 4.f);
	const float Ability_R_SideDamage = GetUniqueAttribute(EAbilityID::Ability_R, "SideArrowsDamage", 55.f);
	const float Ability_R_Angle = GetUniqueAttribute(EAbilityID::Ability_R, "SideArrowsAngle", 10.f);
	const float Ability_R_ArrowSpeed = GetUniqueAttribute(EAbilityID::Ability_R, "ArrowSpeed", 6500.f);
	const float Ability_R_Range = AbilityStatTable.Range;

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Ability_R_Range: %f"), Ability_R_Range), true, true, FLinearColor::Green, 2.0f);


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

	FDamageInformation DamageInformation;
	DamageInformation.AbilityID = EAbilityID::Ability_R;
	DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);
	EnumAddFlags(DamageInformation.AttackEffect, EAttackEffect::AbilityEffects);

	const float AttackSpeed = StatComponent->GetAttackSpeed();
	UAnimMontage* MontageToPlay = GetMontageBasedOnAttackSpeed(AttackSpeed);

	AnimInstance->Montage_Play(MontageToPlay, Ability_LMB_PlayRate);
	PlayMontage_Server(MontageToPlay, Ability_LMB_PlayRate);

	if (UltimateArrowClass)
	{
		// 중앙 화살
		SpawnArrow_Server(UltimateArrowClass, nullptr, FTransform(ArrowSpawnRotation, ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInformation);

		DamageInformation.PhysicalDamage = DamageInformation.PhysicalDamage * Ability_R_SideDamage;

		// 사이드 화살
		SpawnArrow_Server(UltimateArrowClass, nullptr, FTransform(ArrowSpawnRotation + FRotator(0.f, -Ability_R_Angle, 0.f), ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInformation);
		SpawnArrow_Server(UltimateArrowClass, nullptr, FTransform(ArrowSpawnRotation + FRotator(0.f, Ability_R_Angle, 0.f), ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInformation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::ExecuteAbilityR] Cannot spawn arrow because UltimateArrowClass is null."));
	}
}

void ASparrowCharacter::ExecuteAbilityLMB(FVector ArrowSpawnLocation, FRotator ArrowSpawnRotation)
{
	const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB);

	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float Character_AbilityPower = StatComponent->GetAbilityPower();

	const float BaseAttackDamage = AbilityStatTable.AttackDamage;
	const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
	const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
	const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

	const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

	const float Ability_LMB_ArrowSpeed = GetUniqueAttribute(EAbilityID::Ability_LMB, "ArrowSpeed", 6500.f);
	const float Ability_LMB_Range = AbilityStatTable.Range;

	FArrowProperties ArrowProperties;
	ArrowProperties.PierceCount = 0;
	ArrowProperties.Speed = Ability_LMB_ArrowSpeed;
	ArrowProperties.Range = Ability_LMB_Range;

	FDamageInformation DamageInformation;
	DamageInformation.AbilityID = EAbilityID::Ability_LMB;
	DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);
	EnumAddFlags(DamageInformation.AttackEffect, EAttackEffect::OnHit);
	EnumAddFlags(DamageInformation.AttackEffect, EAttackEffect::OnAttack);

	const float AttackSpeed = StatComponent->GetAttackSpeed();
	UAnimMontage* MontageToPlay = GetMontageBasedOnAttackSpeed(AttackSpeed);

	AnimInstance->Montage_Play(MontageToPlay, Ability_LMB_PlayRate);
	PlayMontage_Server(MontageToPlay, Ability_LMB_PlayRate);

	if (BasicArrowClass)
	{
		SpawnArrow_Server(BasicArrowClass, CurrentTarget, FTransform(ArrowSpawnRotation, ArrowSpawnLocation, FVector(1)), ArrowProperties, DamageInformation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::ExecuteAbilityLMB] Cannot spawn arrow because BasicArrowClass is null."));
	}
}

UAnimMontage* ASparrowCharacter::GetMontageBasedOnAttackSpeed(float AttackSpeed)
{
	UAnimMontage* Montage = nullptr;

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_R))
	{
		if (AttackSpeed < 1.f)
		{
			Montage = GetOrLoadMontage("LMB_UltimateMode_Slow", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_LMB_UltimateMode_Slow.Ability_LMB_UltimateMode_Slow"));
		}
		else if (AttackSpeed <= 2.0f)
		{
			Montage = GetOrLoadMontage("LMB_UltimateMode_Med", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_LMB_UltimateMode_Med.Ability_LMB_UltimateMode_Med"));
		}
		else
		{
			Ability_LMB_AnimLength = 0.6f;
			Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);

			Montage = GetOrLoadMontage("LMB_UltimateMode_Fast", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_LMB_UltimateMode_Fast.Ability_LMB_UltimateMode_Fast"));
		}
	}
	else
	{
		if (AttackSpeed < 1.f)
		{
			Montage = GetOrLoadMontage("LMB_Slow", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Slow_Montage.Primary_Fire_Slow_Montage"));
		}
		else if (AttackSpeed <= 2.0f)
		{
			Montage = GetOrLoadMontage("LMB_Med", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Med_Montage.Primary_Fire_Med_Montage"));
		}
		else
		{
			Ability_LMB_AnimLength = 0.6f;
			Ability_LMB_PlayRate = SetAnimPlayRate(Ability_LMB_AnimLength);

			Montage = GetOrLoadMontage("LMB_Fast", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Fast_Montage.Primary_Fire_Fast_Montage"));
		}
	}

	return Montage;
}



/*
	1. UniqueVale[0]: Range		화살 사거리
	2. UniqueVale[0]: Speed		화살 속도
*/
void ASparrowCharacter::Ability_RMB()
{
	if (!ValidateAbilityUsage())
	{
		return;
	}

	bool bAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_RMB);
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::SwitchAction) == false || !bAbilityReady)
	{
		AbilityStatComponent->OnVisibleDescription.Broadcast("The ability is not ready yet.");
		return;
	}

	ServerNotifyAbilityUse(EAbilityID::Ability_RMB, ETriggerEvent::Started);
	PlayMontage("RMB", 1.0f, NAME_None, TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
}

void ASparrowCharacter::Ability_RMB_Canceled()
{
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_RMB) == false)
	{
		return;
	}

	ServerNotifyAbilityUse(EAbilityID::Ability_RMB, ETriggerEvent::Canceled);
	StopAllMontages_Server(0.25f, true);
}


/**
 * RMB 능력을 실행합니다.
 * 캐릭터가 죽었거나 이미 우클릭 능력을 사용하는 경우 능력을 실행하지 않습니다.
 * 능력이 유효한 경우 화살을 발사하고 데미지를 계산하여 적용합니다.
 *
 * [Ability RMB]
 * 1. UniqueAttribute[0]: ArrowSpeed   화살 속도
 * 2. UniqueAttribute[1]: Range        화살 사거리
 */
void ASparrowCharacter::Ability_RMB_Fire()
{
	// 캐릭터가 죽은 상태라면 능력을 실행하지 않습니다.
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Death))
	{
		return;
	}

	// 캐릭터가 이미 우클릭 능력을 사용하는 상태라면 능력을 실행하지 않습니다.
	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_RMB))
	{
		// 능력 사용을 서버에 알립니다.
		ServerNotifyAbilityUse(EAbilityID::Ability_RMB, ETriggerEvent::Triggered);

		// 능력 스탯 테이블을 가져옵니다.
		const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_RMB);
		if (!AbilityStatTable.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_RMB_Fire] AbilityStatTable is null."));
			return;
		}

		// 충돌 지점을 계산합니다.
		FImpactResult ImpactResult = GetImpactPoint(AbilityStatTable.Range);
		FVector ImpactPoint = ImpactResult.ImpactPoint;

		// 화살 스폰 위치와 회전을 계산합니다.
		FVector ArrowSpawnLocation = GetMesh()->GetSocketLocation(FName("arrow_anchor"));
		FRotator ArrowSpawnRotation = UKismetMathLibrary::MakeRotFromX(ImpactPoint - ArrowSpawnLocation);
		FTransform ArrowSpawnTransform(ArrowSpawnRotation, ArrowSpawnLocation, FVector(1));

		// 캐릭터의 공격력과 능력 파워를 가져옵니다.
		const float Character_AttackDamage = StatComponent->GetAttackDamage();
		const float Character_AbilityPower = StatComponent->GetAbilityPower();

		// 능력의 기본 공격력과 능력 파워를 가져옵니다.
		const float BaseAttackDamage = AbilityStatTable.AttackDamage;
		const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
		const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;
		const float AP_PowerScaling = AbilityStatTable.AP_PowerScaling;

		// 최종 데미지를 계산합니다.
		const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * AD_PowerScaling) + (BaseAbilityPower + Character_AbilityPower * AP_PowerScaling);

		// 화살의 속성과 데미지 정보를 설정합니다.
		FArrowProperties ArrowProperties;
		ArrowProperties.PierceCount = GetUniqueAttribute(EAbilityID::Ability_RMB, "PierceCount", 3);
		ArrowProperties.Pierce_DamageReduction = GetUniqueAttribute(EAbilityID::Ability_RMB, "DamageReduction", 10);
		ArrowProperties.Speed = GetUniqueAttribute(EAbilityID::Ability_RMB, "ArrowSpeed", 6500.f);
		ArrowProperties.Range = AbilityStatTable.Range;
		ArrowProperties.Range = AbilityStatTable.Radius;

		FDamageInformation DamageInformation;
		DamageInformation.AbilityID = EAbilityID::Ability_RMB;
		DamageInformation.AddDamage(EDamageType::Magic, FinalDamage);
		EnumAddFlags(DamageInformation.AttackEffect, EAttackEffect::AbilityEffects);

		// 애니메이션을 재생합니다.
		UAnimMontage* Montage = GetOrLoadMontage("RMB", TEXT("/Game/Paragon/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));

		AnimInstance->Montage_JumpToSection(FName(TEXT("Fire")), Montage);
		MontageJumpToSection_Server(Montage, FName(TEXT("Fire")));

		// 화살을 스폰합니다.
		if (::IsValid(PiercingArrowClass))
		{
			SpawnArrow_Server(PiercingArrowClass, nullptr, ArrowSpawnTransform, ArrowProperties, DamageInformation);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_RMB_Fire] Cannot spawn arrow because PiercingArrowClass is null."));
		}
	}
}


void ASparrowCharacter::Ability_Q_CheckHit()
{
	const int32 TimerID = static_cast<uint32>(EAbilityID::Ability_Q);
	const float Radius = GetUniqueAttribute(EAbilityID::Ability_Q, "Radius", 400.f);

	auto TimerCallback = [this, TimerID, Radius, TargetLocation = Ability_Q_DecalLocation]()
		{
			FCollisionQueryParams CollisionParams(NAME_None, false, this);
			TArray<FOverlapResult> ArrowHits;

			FVector CollisionBoxSize = FVector(2 * Radius, 2 * Radius, 3 * Radius); // 박스의 한 변의 길이를 원의 지름으로 설정
			bool bArrowHit = GetWorld()->OverlapMultiByChannel(
				ArrowHits,
				TargetLocation,
				FQuat::Identity,
				ECollisionChannel::ECC_GameTraceChannel3,
				FCollisionShape::MakeBox(CollisionBoxSize),
				CollisionParams
			);

			DrawDebugBox(GetWorld(), TargetLocation, CollisionBoxSize, FQuat::Identity, FColor::Purple, false, 0.0f, 0, 1.0f);
		};
}


void ASparrowCharacter::CancelAbility()
{
	if (!HasAuthority())
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_Q))
	{
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_Q);
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_LMB))
	{
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_LMB);
	}

	if (EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_RMB))
	{
		EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_RMB);
	}

	if (::IsValid(TargetDecalActor))
	{
		TargetDecalActor->Destroy();
	}
}


void ASparrowCharacter::ChangeCameraLength(float TargetLength)
{
	float CurrentArmLength = SpringArmComponent->TargetArmLength;

	//InterpolationAlpha = FMath::Clamp(RotationTimer / AnimLength, 0.f, 1.f);
	SpringArmComponent->TargetArmLength = FMath::InterpEaseInOut(CurrentArmLength, TargetLength, GetWorld()->GetDeltaSeconds(), 0.8);
}

// DestroyAfterSeconds 가 0 이상일 경우 DestroyAfterSeconds 만큼 딜레이 후 스폰된 화살을 파괴합니다.
void ASparrowCharacter::SpawnArrow_Server_Implementation(UClass* SpawnArrowClass, AActor* TargetActor, FTransform SpawnTransform, FArrowProperties InArrowProperties, FDamageInformation InDamageInfomation)
{
	if (SpawnArrowClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ASparrowCharacter::SpawnArrow_Server - Spawn arrow class is nullptr."));
		return;
	}

	AArrowBase* NewArrowActor = Cast<AArrowBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), SpawnArrowClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (NewArrowActor != nullptr)
	{
		NewArrowActor->InitializeArrowActor(TargetActor, InArrowProperties, InDamageInfomation);
		UGameplayStatics::FinishSpawningActor(NewArrowActor, SpawnTransform);
	}
}

void ASparrowCharacter::ExecuteSomethingSpecial()
{
	// GetWorld()가 null인지 확인
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("[ASparrowCharacter::ExecuteSomethingSpecial] World is null."));
		return;
	}

	// 로그 출력: 이 부분은 안정성에 영향이 없지만, GetWorld()가 유효한지 확인 후 호출
	UKismetSystemLibrary::PrintString(GetWorld(), TEXT("[ASparrowCharacter::ExecuteSomethingSpecial] ExecuteSomethingSpecial function called."), true, true, FLinearColor::Red, 2.0f, NAME_None);

	// FDamageInformation 구조체 초기화
	FDamageInformation DamageInformation;
	DamageInformation.AbilityID = EAbilityID::None;

	// CrowdControl 정보 추가: 이 부분은 안정성에 큰 문제를 일으키지 않으나, 필요에 따라 정보 추가 전 유효성 검사 가능
	DamageInformation.CrowdControls.Add(FCrowdControlInformation(EBaseCrowdControl::Stun, 5.0f, 0.0f));

	// ApplyDamage_Server 호출 전 유효성 검사
	if (Controller)
	{
		ApplyDamage_Server(this, DamageInformation, Controller, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ASparrowCharacter::ExecuteSomethingSpecial] Controller is null."));
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

void ASparrowCharacter::OnAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent)
{
	Super::OnAbilityUse(AbilityID, TriggerEvent);

	int32 TimerID = 0;

	if (AbilityID == EAbilityID::Ability_Q)
	{
		if (EnumHasAnyFlags(TriggerEvent, ETriggerEvent::Started))
		{
			EnumAddFlags(CharacterState, EBaseCharacterState::Ability_Q);
			EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
		}
		else if (EnumHasAnyFlags(TriggerEvent, ETriggerEvent::Canceled))
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_Q);
			EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
		}
		else if (EnumHasAnyFlags(TriggerEvent, ETriggerEvent::Triggered))
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_Q);
			EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
		}
	}
	else if (AbilityID == EAbilityID::Ability_E)
	{
		const float Duration = GetUniqueAttribute(EAbilityID::Ability_E, "Duration", 4.0f);
		const float BonusAttakSpeed = GetUniqueAttribute(EAbilityID::Ability_E, "BonusAttakSpeed", 0.f);
		const float BonusMovementSpeed = GetUniqueAttribute(EAbilityID::Ability_E, "BonusMovementSpeed", 0.f);

		EnumAddFlags(CharacterState, EBaseCharacterState::Ability_E);

		StatComponent->ModifyAccumulatedPercentAttackSpeed(BonusAttakSpeed);
		StatComponent->ModifyAccumulatedFlatMovementSpeed(BonusMovementSpeed);

		TimerID = static_cast<int32>(EAbilityID::Ability_E);
		SetGameTimer(AbilityTimer, TimerID,
			[this, TimerID, BonusAttakSpeed, BonusMovementSpeed]()
			{
				EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_E);

				StatComponent->ModifyAccumulatedPercentAttackSpeed(-BonusAttakSpeed);
				StatComponent->ModifyAccumulatedFlatMovementSpeed(-BonusMovementSpeed);

				ClearGameTimer(AbilityTimer, TimerID);
			},
			Duration, false);
	}
	else if (AbilityID == EAbilityID::Ability_R)
	{
		const float Duration = GetUniqueAttribute(EAbilityID::Ability_R, "Duration", 4.0f);
		EnumAddFlags(CharacterState, EBaseCharacterState::Ability_R);

		TimerID = static_cast<int32>(EAbilityID::Ability_R);
		SetGameTimer(AbilityTimer, TimerID,
			[this, TimerID]()
			{
				EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_R);
				ClearGameTimer(AbilityTimer, TimerID);
			},
			Duration, false);
	}
	else if (AbilityID == EAbilityID::Ability_RMB)
	{
		if (TriggerEvent == ETriggerEvent::Started)
		{
			EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
			EnumAddFlags(CharacterState, EBaseCharacterState::Ability_RMB);
		}
		else
		{
			EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
			EnumRemoveFlags(CharacterState, EBaseCharacterState::Ability_RMB);
		}
	}
	else if (AbilityID == EAbilityID::Ability_LMB)
	{
		EnumRemoveFlags(CharacterState, EBaseCharacterState::SwitchAction);
	}
}


void ASparrowCharacter::OnRep_CharacterStateChanged()
{
	Super::OnRep_CharacterStateChanged();

	if (::IsValid(BowParticleSystem))
	{
		EnumHasAnyFlags(CharacterState, EBaseCharacterState::Ability_R) ? BowParticleSystem->Activate() : BowParticleSystem->Deactivate();
	}
}

void ASparrowCharacter::OnRep_CrowdControlStateChanged()
{
	if (EnumHasAnyFlags(CrowdControlState, EBaseCrowdControl::Stun))
	{
		if (::IsValid(TargetDecalActor)) TargetDecalActor->Destroy();
	}
}

void ASparrowCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

}
