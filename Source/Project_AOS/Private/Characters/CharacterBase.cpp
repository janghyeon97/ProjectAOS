#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Components/WidgetComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Structs/CustomCombatData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "CrowdControls/CrowdControlManager.h"
#include "CrowdControls/CrowdControlEffectBase.h"
#include "UI/DamageNumberWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

TMap<FName, UParticleSystem*> ACharacterBase::SharedGameplayParticles;

ACharacterBase::ACharacterBase()
{
	static ConstructorHelpers::FClassFinder<UDamageNumberWidget> DamageNumberClass(TEXT("/Game/ProjectAOS/UI/DamageIndicator/WBP_DamageNumber.WBP_DamageNumber"));
	if (DamageNumberClass.Succeeded()) DamageNumberWidgetClass = DamageNumberClass.Class;

	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bAlwaysRelevant = true;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	StatComponent->OnMovementSpeedChanged.AddDynamic(this, &ACharacterBase::ChangeMovementSpeed);

	EnumAddFlags(CharacterState, EBaseCharacterState::Move);
	EnumAddFlags(CharacterState, EBaseCharacterState::Jump);
	EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);

	CrowdControlManager = UCrowdControlManager::Get();

	// 캐릭터 이동 속도 설정
	ChangeMovementSpeed(0, StatComponent->GetMovementSpeed());
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamSide);
	DOREPLIFETIME(ThisClass, ObjectType);
	DOREPLIFETIME(ThisClass, CharacterState);
	DOREPLIFETIME(ThisClass, CrowdControlState);
	DOREPLIFETIME(ThisClass, bIsDead);
	DOREPLIFETIME(ThisClass, LastHitActor);
	DOREPLIFETIME(ThisClass, bReplicatedUseControllerRotationYaw);
}

//==================== Initialization and Resource Functions ====================//

void ACharacterBase::InitializeCharacterResources()
{
}

void ACharacterBase::SetWidget(class UUserWidgetBase* InUserWidgetBase)
{
}

UAnimMontage* ACharacterBase::GetOrLoadMontage(const FName& Key, const TCHAR* Path)
{
	if (CharacterAnimations.Contains(Key))
	{
		return CharacterAnimations[Key];
	}

	UAnimMontage* Montage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, Path));
	if (Montage)
	{
		CharacterAnimations.Add(Key, Montage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOrLoadMontage: Failed to load montage at path %s"), Path);
	}

	return Montage;
}

UParticleSystem* ACharacterBase::GetOrLoadParticle(const FName& Key, const TCHAR* Path)
{
	if (CharacterParticles.Contains(Key))
	{
		return CharacterParticles[Key];
	}

	UParticleSystem* ParticleSystem = Cast<UParticleSystem>(StaticLoadObject(UParticleSystem::StaticClass(), nullptr, Path));
	if (ParticleSystem)
	{
		CharacterParticles.Add(Key, ParticleSystem);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOrLoadParticle: Failed to load particle system at path %s"), Path);
	}
	return ParticleSystem;
}

UStaticMesh* ACharacterBase::GetOrLoadMesh(const FName& Key, const TCHAR* Path)
{
	if (CharacterMeshes.Contains(Key))
	{
		return CharacterMeshes[Key];
	}

	UStaticMesh* StaticMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, Path));
	if (StaticMesh)
	{
		CharacterMeshes.Add(Key, StaticMesh);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOrLoadMesh: Failed to load static mesh at path %s"), Path);
	}
	return StaticMesh;
}

UParticleSystem* ACharacterBase::GetOrLoadSharedParticle(const FName& Key, const TCHAR* Path)
{
	if (SharedGameplayParticles.Contains(Key))
	{
		return SharedGameplayParticles[Key];
	}

	UParticleSystem* ParticleSystem = Cast<UParticleSystem>(StaticLoadObject(UParticleSystem::StaticClass(), nullptr, Path));
	if (ParticleSystem)
	{
		SharedGameplayParticles.Add(Key, ParticleSystem);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOrLoadParticle: Failed to load particle system at path %s"), Path);
	}
	return ParticleSystem;
}

//==================== Ability Execution Functions ====================//

void ACharacterBase::Ability_Q()
{
}

void ACharacterBase::Ability_E()
{
}

void ACharacterBase::Ability_R()
{
}

void ACharacterBase::Ability_LMB()
{
}

void ACharacterBase::Ability_RMB()
{
}

//==================== Ability Check Functions ====================//

void ACharacterBase::Ability_Q_CheckHit()
{
}

void ACharacterBase::Ability_E_CheckHit()
{
}

void ACharacterBase::Ability_R_CheckHit()
{
}

void ACharacterBase::Ability_LMB_CheckHit()
{
}

void ACharacterBase::Ability_RMB_CheckHit()
{
}


//==================== Ability Canceled Functions ====================//

void ACharacterBase::CancelAbility()
{
}

void ACharacterBase::Ability_Q_Canceled()
{
}

void ACharacterBase::Ability_E_Canceled()
{
}

void ACharacterBase::Ability_R_Canceled()
{
}

void ACharacterBase::Ability_LMB_Canceled()
{
}

void ACharacterBase::Ability_RMB_Canceled()
{
}

//==================== Damage-related Functions ====================//

bool ACharacterBase::ReceiveDamage(FDamageInformation DamageInformation, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamageAmount = 0;

	if (OnPreDamageCalculationEvent.IsBound())
	{
		OnPreDamageCalculationEvent.Broadcast(DamageInformation);
	}

	if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Physical) || EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Critical))
	{
		if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Critical))
		{
			float ReducedCriticalDamage = DamageInformation.PhysicalDamage * (100 / (100 + StatComponent->GetDefensePower()));
			FinalDamageAmount += ReducedCriticalDamage;
			DamageInformation.PhysicalDamage = ReducedCriticalDamage;
		}
		else
		{
			float ReducedPhysicalDamage = DamageInformation.PhysicalDamage * (100 / (100 + StatComponent->GetDefensePower()));
			FinalDamageAmount += ReducedPhysicalDamage;
			DamageInformation.PhysicalDamage = ReducedPhysicalDamage;
		}
	}

	if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Magic))
	{
		float ReducedMagicDamage = DamageInformation.MagicDamage * (100 / (100 + StatComponent->GetMagicResistance()));
		FinalDamageAmount += ReducedMagicDamage;
		DamageInformation.MagicDamage = ReducedMagicDamage;
	}

	if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::TrueDamage))
	{
		FinalDamageAmount += DamageInformation.TrueDamage;
	}

	if (OnPostDamageCalculationEvent.IsBound())
	{
		OnPostDamageCalculationEvent.Broadcast(FinalDamageAmount);
	}

	SpawnDamageWidget_Client(this, FinalDamageAmount, DamageInformation);

	UE_LOG(LogTemp, Warning, TEXT("%s Received %f Damages from %s, reduced CurrentHP from %f to %f"),
		*this->GetName(), FinalDamageAmount, *DamageCauser->GetName(), StatComponent->GetCurrentHP(), StatComponent->GetCurrentHP() - FinalDamageAmount);

	ACharacterBase* DamageCauserActor = Cast<ACharacterBase>(DamageCauser);
	if (::IsValid(DamageCauserActor))
	{
		DamageCauserActor->SpawnDamageWidget_Client(this, FinalDamageAmount, DamageInformation);
		LastHitActor = DamageCauserActor;
	}

	StatComponent->ModifyCurrentHP(-FinalDamageAmount);

	return true;
}

void ACharacterBase::ApplyCriticalHitDamage(FDamageInformation& DamageInformation)
{
	float CriticalChance = static_cast<float>(StatComponent->GetCriticalChance()) / 100;
	if (CriticalChance <= 0)
	{
		return;
	}

	float ExpectedCriticalHits = TotalAttacks * CriticalChance;
	float ActualCriticalHits = static_cast<float>(CriticalHits);

	float Adjustment = (ExpectedCriticalHits - ActualCriticalHits) / (TotalAttacks + 1);
	float AdjustedCriticalChance = CriticalChance + Adjustment;

	bool bIsCriticalHit = (FMath::FRand() <= AdjustedCriticalChance);

	if (bIsCriticalHit)
	{
		CriticalHits++;

		if (::IsValid(StatComponent))
		{
			float AttackDamage = StatComponent->GetAttackDamage();
			DamageInformation.AddDamage(EDamageType::Critical, AttackDamage * 0.7);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::ApplyCriticalHitDamage - StatComponent is not valid."));
		}
	}
	TotalAttacks++;
}

void ACharacterBase::ApplyDamage_Server_Implementation(ACharacterBase* Enemy, FDamageInformation DamageInformation, AController* EventInstigator, AActor* DamageCauser)
{
	if (!::IsValid(Enemy))
	{
		return;
	}

	if (DamageInformation.AbilityID == EAbilityID::Ability_LMB)
	{
		if (EnumHasAnyFlags(CrowdControlState, EBaseCrowdControl::Blind))
		{
			// To do
			return;
		}
	}

	bool bResult = Enemy->ValidateHit(DamageInformation.AbilityID);
	if (!bResult)
	{
		return;
	}

	if (EnumHasAnyFlags(DamageInformation.AttackEffect, EAttackEffect::OnHit) && OnHitEventTriggered.IsBound())
	{
		OnHitEventTriggered.Broadcast(DamageInformation);
	}

	if (EnumHasAnyFlags(DamageInformation.AttackEffect, EAttackEffect::OnAttack) && OnAttackEventTriggered.IsBound())
	{
		OnAttackEventTriggered.Broadcast(DamageInformation);
	}

	if (EnumHasAnyFlags(DamageInformation.AttackEffect, EAttackEffect::AbilityEffects) && OnAbilityEffectsEventTriggered.IsBound())
	{
		OnAbilityEffectsEventTriggered.Broadcast(DamageInformation);
	}

	Enemy->ReceiveDamage(DamageInformation, EventInstigator, DamageCauser);

	if (DamageInformation.CrowdControls.Num() > 0)
	{
		for (auto& CrowdControl : DamageInformation.CrowdControls)
		{
			Enemy->ApplyCrowdControlEffect(CrowdControl);
		}
	}
}

//==================== Crowd Control Functions ====================//

void ACharacterBase::ApplyCrowdControlEffect(FCrowdControlInformation CrowdControlInfo)
{
	EBaseCrowdControl Type = CrowdControlInfo.Type;

	// CrowdControlManager가 유효한지 확인
	if (!::IsValid(CrowdControlManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("CrowdControlManager is null. Cannot apply crowd control effect of type: %d"), static_cast<int32>(Type));
		return;
	}

	// CrowdControlManager에서 효과 클래스 가져오기
	TSubclassOf<UCrowdControlEffectBase> EffectClass = CrowdControlManager->GetEffectClass(Type);
	if (!::IsValid(EffectClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("No effect class found for crowd control type: %d"), static_cast<int32>(Type));
		return;
	}

	// 이미 효과가 활성화된 경우 처리
	if (UCrowdControlEffectBase* ExistingEffect = ActiveEffects.FindRef(Type))
	{
		if (ExistingEffect)
		{
			ExistingEffect->ApplyEffect(this, CrowdControlInfo.Duration, CrowdControlInfo.Percent);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Existing effect for type %d is invalid, removing from ActiveEffects"), static_cast<int32>(Type));
			ActiveEffects.Remove(Type);
		}
	}

	// 객체 풀에서 효과 객체 가져오기
	UCrowdControlEffectBase* NewEffect = CrowdControlManager->GetEffect(Type);
	if (!::IsValid(NewEffect))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve new crowd control effect object from pool for type %d"), static_cast<int32>(Type));
		return;
	}

	NewEffect->ApplyEffect(this, CrowdControlInfo.Duration, CrowdControlInfo.Percent);
	ActiveEffects.Add(Type, NewEffect);
}



void ACharacterBase::ApplyCrowdControl_Server_Implementation(ACharacterBase* Enemy, FCrowdControlInformation CrowdControlInfo)
{
	if (HasAuthority())
	{
		
	}
}

void ACharacterBase::ApplyCrowdControl_NetMulticast_Implementation(ACharacterBase* Enemy, FCrowdControlInformation CrowdControlInfo)
{
	if (::IsValid(Enemy))
	{
		
	}
}

//==================== State Functions ====================//

void ACharacterBase::LogCharacterState(EBaseCharacterState State, const FString& Context)
{
	FString CharacterStateString;
	UEnum* EnumPtr = StaticEnum<EBaseCharacterState>();
	if (EnumPtr)
	{
		for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
		{
			int64 Value = EnumPtr->GetValueByIndex(i);
			if (EnumHasAnyFlags(State, static_cast<EBaseCharacterState>(Value)) && Value != static_cast<int64>(EBaseCharacterState::None))
			{
				CharacterStateString += EnumPtr->GetNameStringByIndex(i) + TEXT(" ");
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[%s::%s] Actor: %s - CharacterState: %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"),
			*Context, *GetName(), *CharacterStateString);
	}
}

void ACharacterBase::ModifyCharacterState_Implementation(ECharacterStateOperation Operation, EBaseCharacterState StateFlag)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] ModifyCharacterState_Implementation called without authority."));
		return;
	}

	switch (Operation)
	{
	case ECharacterStateOperation::Add:
		EnumAddFlags(CharacterState, StateFlag);
		break;
	case ECharacterStateOperation::Remove:
		EnumRemoveFlags(CharacterState, StateFlag);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown Operation: %d"), static_cast<int32>(Operation));
		break;
	}
}

//==================== Movement Functions ====================//

void ACharacterBase::ChangeMovementSpeed(float InOldMS, float InNewMS)
{
	GetCharacterMovement()->MaxWalkSpeed = InNewMS;
}

//==================== Delegate Functions ====================//

void ACharacterBase::OnRep_CharacterStateChanged()
{
}

void ACharacterBase::OnRep_CrowdControlStateChanged()
{
}

void ACharacterBase::OnPreCalculateDamage(float& AttackDamage, float& AbilityPower)
{
}

void ACharacterBase::OnPreDamageReceived(float FinalDamage)
{
}

void ACharacterBase::OnRep_UseControllerRotationYaw()
{
	bUseControllerRotationYaw = bReplicatedUseControllerRotationYaw;
}

//==================== Ability Stat Component Getter ====================//

UStatComponent* ACharacterBase::GetStatComponent() const
{
	return StatComponent;
}

UAbilityStatComponent* ACharacterBase::GetAbilityStatComponent() const
{
	return AbilityStatComponent;
}


//==================== Miscellaneous Functions ====================//

bool ACharacterBase::ValidateHit(EAbilityID AbilityID)
{
	if (AbilityID == EAbilityID::Ability_LMB)
	{
		return true;
	}
	else
	{
		bool bResult = true;
		if (OnReceiveDamageEnteredEvent.IsBound())
		{
			OnReceiveDamageEnteredEvent.Broadcast(bResult);
		}
		return bResult;
	}
}

float ACharacterBase::GetUniqueAttribute(EAbilityID AbilityID, const FString& Key, float DefaultValue) const
{
	if (AbilityStatComponent)
	{
		return AbilityStatComponent->GetUniqueValue(AbilityID, Key, DefaultValue);
	}
	return DefaultValue;
}

float ACharacterBase::SetAnimPlayRate(const float AnimLength)
{
	if (!::IsValid(StatComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("SetAnimPlayRate: StatComponent is null."));
		return 1.0f;
	}

	float CurrentAttackSpeed = StatComponent->GetAttackSpeed();
	float AttackIntervalTime = 1.0f / CurrentAttackSpeed;

	float PlayRate = (AttackIntervalTime < AnimLength) ? (AnimLength / AttackIntervalTime) : 1.0f;

	const float MinPlayRate = 0.5f;
	const float MaxPlayRate = 2.0f;
	PlayRate = FMath::Clamp(PlayRate, MinPlayRate, MaxPlayRate);

	return PlayRate;
}

//==================== Particle Functions ====================//

void ACharacterBase::SpawnRootedParticleAtLocation_Server_Implementation(UParticleSystem* Particle, FTransform Transform)
{
	if (HasAuthority())
	{
		SpawnRootedParticleAtLocation_Multicast(Particle, Transform);
	}
}

void ACharacterBase::SpawnRootedParticleAtLocation_Multicast_Implementation(UParticleSystem* Particle, FTransform Transform)
{
	if (!HasAuthority())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), true, EPSCPoolMethod::None, true);
	}
}

void ACharacterBase::SpawnAttachedParticleAtLocation_Server_Implementation(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType)
{
	if (HasAuthority())
	{
		SpawnAttachedParticleAtLocation_Multicast(Particle, AttachToComponent, Transform, LocationType);
	}
}

void ACharacterBase::SpawnAttachedParticleAtLocation_Multicast_Implementation(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType)
{
	if (!HasAuthority())
	{
		UGameplayStatics::SpawnEmitterAttached(
			Particle, AttachToComponent, "Name_None", Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), LocationType, true, EPSCPoolMethod::None, true
		);
	}
}

//==================== Mesh Functions ====================//

void ACharacterBase::SpawnAttachedMeshAtLocation_Server_Implementation(UStaticMesh* MeshToSpawn, FVector Location, float Duration)
{
	if (HasAuthority())
	{
		SpawnAttachedMeshAtLocation_Multicast(MeshToSpawn, Location, Duration);
	}
}

void ACharacterBase::SpawnAttachedMeshAtLocation_Multicast_Implementation(UStaticMesh* MeshToSpawn, FVector Location, float Duration)
{
	if (!HasAuthority() && ::IsValid(MeshToSpawn))
	{
		FTimerHandle NewTimerHandle;

		UStaticMeshComponent* NewMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());
		if (::IsValid(NewMeshComponent))
		{
			FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);

			NewMeshComponent->RegisterComponent();
			NewMeshComponent->AttachToComponent(GetRootComponent(), AttachmentRules);
			NewMeshComponent->CreationMethod = EComponentCreationMethod::Instance;
			NewMeshComponent->SetStaticMesh(MeshToSpawn);
			NewMeshComponent->SetCollisionProfileName("CharacterMesh");

			GetWorldTimerManager().SetTimer(
				NewTimerHandle,
				[MeshComponent = NewMeshComponent]()
				{
					if (MeshComponent)
					{
						MeshComponent->DestroyComponent();
					}
				},
				Duration,
				false
			);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create new mesh component."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid MeshToSpawn or unauthorized access."));
	}
}

//==================== Actor Functions ====================//

void ACharacterBase::SpawnActorAtLocation_Server_Implementation(UClass* SpawnActor, FTransform SpawnTransform)
{
	if (HasAuthority())
	{
		if (!SpawnActor)
		{
			return;
		}

		GetWorld()->SpawnActor<AActor>(SpawnActor, SpawnTransform);
	}
}


//==================== Montage-related functions ====================//

void ACharacterBase::PlayMontage(const FString& MontageName, float PlayRate, FName StartSectionName, const TCHAR* Path)
{
	UAnimMontage* Montage = GetOrLoadMontage(FName(*MontageName), Path);
	if (!Montage)
	{
		return;
	}

	PlayAnimMontage(Montage, PlayRate, StartSectionName);
	PlayMontage_Server(Montage, PlayRate, StartSectionName);
}


void ACharacterBase::PlayMontage_Server_Implementation(UAnimMontage* Montage, float PlayRate, FName SectionName, bool bApplyToLocalPlayer)
{
	PlayMontage_NetMulticast(Montage, PlayRate, SectionName, bApplyToLocalPlayer);
}

void ACharacterBase::PlayMontage_NetMulticast_Implementation(UAnimMontage* Montage, float PlayRate, FName SectionName, bool bApplyToLocalPlayer)
{
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayMontage_NetMulticast_Implementation: Montage is null."));
		return;
	}

	if (HasAuthority())
	{
		//UE_LOG(LogTemp, Log, TEXT("[%s] PlayMontage_NetMulticast_Implementation: Skipping montage '%s' on server."), *GetName(), *Montage->GetName());
		return;
	}

	if (!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (bApplyToLocalPlayer)
		{
			//UE_LOG(LogTemp, Log, TEXT("[%s] PlayMontage_NetMulticast_Implementation: Playing montage '%s''%s' on owning client."), *GetName(), *Montage->GetName(), *SectionName.ToString());
			PlayAnimMontage(Montage, PlayRate, SectionName);
		}
		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("[%s] PlayMontage_NetMulticast_Implementation: Playing montage '%s''%s' on %s."), *GetName(), *Montage->GetName(), *SectionName.ToString(), HasAuthority() ? TEXT("server") : TEXT("client"));
	PlayAnimMontage(Montage, PlayRate, SectionName);
}


void ACharacterBase::StopAllMontages_Server_Implementation(float BlendOut, bool bApplyToLocalPlayer)
{
	if (HasAuthority())
	{
		StopAllMontages_NetMulticast(BlendOut, bApplyToLocalPlayer);
	}	
}

void ACharacterBase::StopAllMontages_NetMulticast_Implementation(float BlendOut, bool bApplyToLocalPlayer)
{
	if (!::IsValid(AnimInstance))
	{
		return;
	}

	if (HasAuthority())
	{
		//UE_LOG(LogTemp, Log, TEXT("StopAllMontages_NetMulticast_Implementation: Skipping on server."));
		return;
	}

	if (!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (bApplyToLocalPlayer)
		{
			//UE_LOG(LogTemp, Log, TEXT("StopAllMontages_NetMulticast_Implementation: Stop all montage on owning client."));
			AnimInstance->StopAllMontages(BlendOut);
		}
		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("StopAllMontages_NetMulticast_Implementation: Stop all montage on %s."), HasAuthority() ? TEXT("server") : TEXT("client"));
	AnimInstance->StopAllMontages(BlendOut);
}

void ACharacterBase::MontageJumpToSection_Server_Implementation(const UAnimMontage* Montage, FName SectionName)
{
	MontageJumpToSection_NetMulticast(Montage, SectionName);
}

void ACharacterBase::MontageJumpToSection_NetMulticast_Implementation(const UAnimMontage* Montage, FName SectionName)
{
	if ((!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(this, 0)))
	{
		return;
	}

	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->Montage_JumpToSection(SectionName, Montage);
}


void ACharacterBase::PauseMontage_NetMulticast_Implementation()
{
	/*if ((!HasAuthority() && GetOwner() == UGameplayStatics::GetPlayerController(this, 0)) || IsLocallyControlled())
	{
		return;
	}*/

	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->Montage_Pause(AnimInstance->GetCurrentActiveMontage());
}

void ACharacterBase::ResumeMontage_NetMulticast_Implementation()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	// 애니메이션 재개
	if (AnimInstance && !AnimInstance->Montage_IsPlaying(AnimInstance->GetCurrentActiveMontage()))
	{
		AnimInstance->Montage_Resume(AnimInstance->GetCurrentActiveMontage());
	}
}

// -----------------------------------------------------------

void ACharacterBase::SpawnDamageWidget_Client_Implementation(AActor* Target, const float DamageAmount, const FDamageInformation& DamageInformation)
{
	// UWidgetComponent를 동적으로 생성
	UWidgetComponent* DamageWidgetComponent = NewObject<UWidgetComponent>(Target);
	if (DamageWidgetComponent)
	{
		DamageWidgetComponent->SetupAttachment(Target->GetRootComponent());

		// 랜덤 오프셋 생성 (위치가 약간 다르게 스폰되도록 설정)
		FVector RandomOffset(FMath::FRandRange(-30.0f, 30.0f), FMath::FRandRange(-30.0f, 30.0f), FMath::FRandRange(40, 80.0f));

		// 위젯을 캐릭터의 머리 위에 배치 (랜덤 오프셋 적용)
		DamageWidgetComponent->SetRelativeLocation(RandomOffset);
		DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);    // 위젯이 화면 공간에 표시되도록 설정
		DamageWidgetComponent->SetDrawAtDesiredSize(true);              // 위젯 크기를 자동 조정
		DamageWidgetComponent->RegisterComponent();                    // 컴포넌트를 등록하여 활성화

		// 위젯이 다른 모든 것보다 앞에 있도록 ZOrder를 크게 설정
		DamageWidgetComponent->SetInitialLayerZOrder(0);

		// DamageNumberWidgetClass 설정
		if (DamageNumberWidgetClass)
		{
			DamageWidgetComponent->SetWidgetClass(DamageNumberWidgetClass);
			UDamageNumberWidget* DamageWidget = Cast<UDamageNumberWidget>(DamageWidgetComponent->GetUserWidgetObject());
			if (DamageWidget)
			{
				FLinearColor TextColor;
				float TextScale = 1.0f;
				FString LogMessage;

				if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Physical))
				{
					TextColor = FLinearColor(255.0f / 255.0f, 22.0f / 255.0f, 15.0f / 255.0f, 255.0f / 255.0f);
					TextScale = 0.3f;
					LogMessage = TEXT("Damage Type: Physical, Color: (255, 22, 15), Scale: 0.3f");
				}
				else if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Magic))
				{
					TextColor = FLinearColor(26.0f / 255.0f, 29.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
					TextScale = 0.8f;
					LogMessage = TEXT("Damage Type: Magic, Color: (26, 29, 255), Scale: 0.8f");
				}
				else if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Critical))
				{
					TextColor = FLinearColor::Red;
					TextScale = 1.0f;
					LogMessage = TEXT("Damage Type: Critical, Color: Red, Scale: 1.0f");
				}
				else if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::TrueDamage))
				{
					TextColor = FLinearColor(145.0f / 255.0f, 145.0f / 255.0f, 145.0f / 255.0f, 255.0f / 255.0f);
					TextScale = 1.0f;
					LogMessage = TEXT("Damage Type: TrueDamage, Color: (145, 145, 145), Scale: 1.0f");
				}
				else
				{
					TextColor = FLinearColor::White;
					TextScale = 1.0f;
					LogMessage = TEXT("Damage Type: None, Color: White, Scale: 1.0f");
				}

				// 화면에 로그 메시지 출력
				UKismetSystemLibrary::PrintString(this, LogMessage, true, true, TextColor, 2.0f);

				DamageWidget->SetDamageAmount(DamageAmount, TextColor, TextScale);
				DamageWidgetQueue.Enqueue(DamageWidgetComponent);

				// 애니메이션이 끝나면 컴포넌트를 제거하기 위한 콜백 설정
				if (UWidgetAnimation* Anim = DamageWidget->GetFadeOutAnimation())
				{
					FWidgetAnimationDynamicEvent EndEvent;
					EndEvent.BindDynamic(this, &ACharacterBase::OnDamageWidgetAnimationFinished);
					DamageWidget->BindToAnimationFinished(Anim, EndEvent);
					//DamageWidgetComponent->SetIsReplicated(false); 

					// 애니메이션 재생
					DamageWidget->PlayAnimation(Anim);
				}
			}
		}
	}
}


void ACharacterBase::OnDamageWidgetAnimationFinished()
{
	UWidgetComponent* DamageWidgetComponent = nullptr;

	// 큐에서 위젯 컴포넌트를 꺼내 제거
	if (DamageWidgetQueue.Dequeue(DamageWidgetComponent))
	{
		if (DamageWidgetComponent)
		{
			DamageWidgetComponent->DestroyComponent(); 
		}
	}
}