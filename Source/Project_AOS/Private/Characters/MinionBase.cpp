// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/MinionBase.h"
#include "Controllers/MinionAIController.h"
#include "Components/MinionStatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/AOSGameMode.h"
#include "Game/AOSGameInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animations/MinionAnimInstance.h"
#include "UI/UW_HPBar.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Null.h"

AMinionBase::AMinionBase()
{
	AIControllerClass = AMinionAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bReplicates = true;
	SetReplicateMovement(true);

	StatComponent = CreateDefaultSubobject<UMinionStatComponent>(TEXT("MinionStatComponent"));
	AbilityStatComponent = CreateDefaultSubobject<UAbilityStatComponent>(TEXT("MinionAbilityStatComponent"));

	if (HasAuthority())
	{
		StatComponent->SetIsReplicated(true);
		AbilityStatComponent->SetIsReplicated(true);
	}

	WidgetComponent = CreateDefaultSubobject<UCharacterWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetRootComponent());
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));

	static ConstructorHelpers::FClassFinder<UUserWidgetBase> StateBarWidgetRef(TEXT("/Game/ProjectAOS/UI/WBP_HPBar.WBP_HPBar"));
	if (StateBarWidgetRef.Succeeded())
	{
		WidgetComponent->SetWidgetClass(StateBarWidgetRef.Class);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(200.0f, 35.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement())
	{
		CharacterMovementComp->bOrientRotationToMovement = false;
		CharacterMovementComp->bUseControllerDesiredRotation = true;
		CharacterMovementComp->RotationRate = FRotator(0.f, 480.f, 0.f);
		CharacterMovementComp->MaxWalkSpeed = 500.f;

		CharacterMovementComp->bUseRVOAvoidance = true;
		CharacterMovementComp->AvoidanceWeight = 0.5;
		CharacterMovementComp->AvoidanceConsiderationRadius = 100.f; // 충돌 회피 고려 반경
		CharacterMovementComp->MaxAcceleration = 2048.0f; // 충분한 가속도를 부여
		CharacterMovementComp->bRequestedMoveUseAcceleration = true; // RVO를 통해 직접적인 이동 요청 허용

		CharacterMovementComp->SetAvoidanceGroup(1);
		CharacterMovementComp->SetGroupsToAvoid(1);
		CharacterMovementComp->SetGroupsToIgnore(0);
	}

	// NavMesh 강제 업데이트
	/*UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		NavSys->Build(); // 전체 NavMesh를 다시 빌드
	}*/


	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		float CharacterHalfHeight = 65.f;
		float CharacterRadius = 60.f;

		CapsuleComp->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);
		CapsuleComp->SetAreaClassOverride(UNavArea_Null::StaticClass());
		CapsuleComp->SetCanEverAffectNavigation(true);
		CapsuleComp->bDynamicObstacle = true;
	}
	

	FVector PivotPosition(0.f, 0.f, -60.f);
	FRotator PivotRotation(0.f, 0.f, -90.f);
	GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;

	ObjectType = EObjectType::Minion;
	GoldBounty = 0;
	ExpBounty = 0;
	ReplicatedSkeletalMesh = nullptr;
	SplineActor = nullptr;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Minion"));
}

void AMinionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMinionBase, ReplicatedSkeletalMesh);
}

void AMinionBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == false)
		{
			StatComponent->OnOutOfCurrentHP.AddDynamic(this, &ThisClass::OnCharacterDeath);
		}

		UParticleSystem* Particle = GetOrLoadParticle("Spawn", TEXT("/Game/ParagonMinions/FX/Particles/Minions/Shared/P_MinionSpawn.P_MinionSpawn"));
		if (Particle)
		{
			SpawnRootedParticleAtLocation_Server(Particle, FTransform(FRotator(0), GetActorLocation(), FVector(1)));
		}
	}

	AnimInstance = Cast<UMinionAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::MontageEnded);
	}
}

void AMinionBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (!GameInstance) return;

		if (ICharacterDataProviderInterface* Provider = Cast<ICharacterDataProviderInterface>(GameInstance->GetDataProvider(EObjectType::Minion)))
		{
			UMinionStatComponent* MinionStatComponent = Cast<UMinionStatComponent>(StatComponent);
			if (MinionStatComponent)
			{
				MinionStatComponent->InitStatComponent(Provider, MinionType);

				MinionStatComponent->OnOutOfCurrentHP.AddDynamic(this, &AMinionBase::OnCharacterDeath);
			}

			UAbilityStatComponent* MinionAbilityStatComponent = Cast<UAbilityStatComponent>(AbilityStatComponent);
			if (MinionAbilityStatComponent)
			{
				AbilityStatComponent->InitAbilityStatComponent(Provider, MinionStatComponent, MinionType);
				AbilityStatComponent->InitializeAbility(EAbilityID::Ability_LMB, 1);
			}
		}
	}
}

void AMinionBase::InitializeCharacterResources()
{
	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Invalid GameInstance."));
		return;
	}

	ICharacterDataProviderInterface* Provider = Cast<ICharacterDataProviderInterface>(GameInstance->GetDataProvider(EObjectType::Minion));
	if (!Provider)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Invalid DataProvider."));
		return;
	}

	CharacterAnimations = Provider->GetCharacterMontagesMap(MinionType);
	CharacterParticles = Provider->GetCharacterParticlesMap(MinionType);
	CharacterMeshes = Provider->GetCharacterMeshesMap(MinionType);
}

void AMinionBase::SetWidget(UUserWidgetBase* InUserWidgetBase)
{
	HPBar = Cast<UUW_HPBar>(InUserWidgetBase);

	if (::IsValid(HPBar) && ::IsValid(StatComponent))
	{
		HPBar->SetBorderColor(FLinearColor(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f));

		StatComponent->OnMaxHPChanged.AddDynamic(HPBar, &UUW_HPBar::OnMaxHPChanged);
		StatComponent->OnCurrentHPChanged.AddDynamic(HPBar, &UUW_HPBar::OnCurrentHPChanged);

		HPBar->SetMaxHP(StatComponent->GetMaxHP());
		HPBar->OnCurrentHPChanged(0, StatComponent->GetMaxHP());
	}
}

void AMinionBase::OnCharacterDeath()
{
	if (!LastHitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] LastHitActor is null."));
		return;
	}

	RelativeDirection = GetRelativeDirection(LastHitActor);
	if (RelativeDirection == -1 || RelativeDirection == 0)
	{
		RelativeDirection = 1;
	}

	UAnimMontage* DeathMontage = GetOrLoadMontage("Death", TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Death_Montage.Death_Montage"));
	if (!DeathMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] Failed to load DeathMontage."));
		SetActorHiddenInGame(true);
	}
	else
	{
		FName SectionName = FName(*FString::Printf(TEXT("Death%d"), RelativeDirection));
		PlayAnimMontage(DeathMontage, 1.0f, SectionName);
		PlayMontage_Server(DeathMontage, 1.0f, SectionName);
	}

	StatComponent->OnOutOfCurrentHP.RemoveDynamic(this, &ThisClass::OnCharacterDeath);

	OnCharacterDeath_Multicast();
	EnumAddFlags(CharacterState, EBaseCharacterState::Death);

	// Disable character collision and movement
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	// Disable actor
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	AAOSGameMode* GM = Cast<AAOSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] Failed to get game mode."));
		return;
	}

	TArray<ACharacterBase*> NearbyEnemies;
	FindNearbyPlayers(NearbyEnemies, this->TeamSide == ETeamSideBase::Blue ? ETeamSideBase::Red : ETeamSideBase::Blue, ExperienceShareRadius);

	ACharacterBase* Eliminator = Cast<ACharacterBase>(LastHitActor);
	if (Eliminator && EnumHasAnyFlags(Eliminator->ObjectType, EObjectType::Player))
	{
		GM->AddCurrencyToPlayer(Eliminator, GoldBounty);
	}

	DistributeExperience(Eliminator, NearbyEnemies);
}

void AMinionBase::OnCharacterDeath_Multicast_Implementation()
{
	if (HasAuthority())
	{
		return;
	}

	HPBar->SetVisibility(ESlateVisibility::Hidden);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMinionBase::EnableRagdoll()
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetPhysicsBlendWeight(1.0f);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
		MovementComponent->StopMovementImmediately();
	}
}

void AMinionBase::ApplyDirectionalImpulse()
{
	FVector ImpulseDirection = FVector::ZeroVector;

	// 맞은 방향에 따라 임펄스 방향 설정
	switch (RelativeDirection)
	{
	case 1: // 앞
		ImpulseDirection = -GetActorForwardVector();
		break;
	case 2: // 뒤
		ImpulseDirection = GetActorForwardVector();
		break;
	case 3: // 오른쪽
		ImpulseDirection = -GetActorRightVector();
		break;
	case 4: // 왼쪽
		ImpulseDirection = GetActorRightVector();
		break;
	default:
		break;
	}

	FVector Impulse = ImpulseDirection * ImpulseStrength;

	GetMesh()->AddImpulse(Impulse, NAME_None, true);

	// Draw a debug directional arrow to represent the impulse
	FVector StartLocation = GetMesh()->GetComponentLocation();
	FVector EndLocation = StartLocation + Impulse;
	DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 100.0f, FColor::Red, false, 5.0f, 0, 5.0f);
}

void AMinionBase::MulticastApplyImpulse_Implementation(FVector Impulse)
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{

	}
}

void AMinionBase::StartFadeOut()
{
	CurrentFadeDeath += (0.05f / FadeOutDuration);
	if (CurrentFadeDeath >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		CurrentFadeDeath = 1.0f;
		if(HasAuthority()) Destroy(true, true);
	}

	// 메쉬의 머티리얼 투명도 업데이트
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent)
	{
		int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 i = 0; i < MaterialCount; i++)
		{
			UMaterialInstanceDynamic* DynMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(i);
			if (DynMaterial)
			{
				DynMaterial->SetScalarParameterValue(FName("FadeOut"), CurrentFadeDeath);
			}
		}
	}
}

void AMinionBase::DistributeExperience(ACharacterBase* Eliminator, const TArray<ACharacterBase*>& NearbyEnemies)
{
	AAOSGameMode* GM = Cast<AAOSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
		return;
	}

	int32 NearbyAlliesCount = NearbyEnemies.Num() + (Eliminator ? 1 : 0);
	float ShareFactorValue = 1.0f;

	if (NearbyAlliesCount > 0)
	{
		if (ShareFactor.Contains(NearbyAlliesCount))
		{
			ShareFactorValue = ShareFactor[NearbyAlliesCount];
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No share factor defined for %d players"), NearbyAlliesCount);
		}

		float TotalExp = ExpBounty * ShareFactorValue;
		float SharedExpBounty = TotalExp / NearbyAlliesCount;

		if (Eliminator && EnumHasAnyFlags(Eliminator->ObjectType, EObjectType::Player))
		{
			GM->AddExpToPlayer(Eliminator, SharedExpBounty);
		}

		for (auto& Player : NearbyEnemies)
		{
			GM->AddExpToPlayer(Player, SharedExpBounty);
		}
	}
}

void AMinionBase::FindNearbyPlayers(TArray<ACharacterBase*>& PlayerCharacters, ETeamSideBase InTeamSide, float Distance)
{
	// Overlap detection
	TArray<FOverlapResult> OutHits;
	FVector CollisionBoxSize = FVector(2 * Distance, 2 * Distance, 2 * Distance); // 박스의 한 변의 길이를 원의 지름으로 설정
	FVector OverlapLocation = GetActorLocation();
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bResult = GetWorld()->OverlapMultiByChannel(
		OutHits,
		OverlapLocation,
		FQuat::Identity,
		ECC_GameTraceChannel4,
		FCollisionShape::MakeBox(CollisionBoxSize),
		Params
	);

	if (bResult)
	{
		for (const auto& OutHit : OutHits)
		{
			AActor* OverlappedActor = OutHit.GetActor();
			if (!::IsValid(OverlappedActor) || OverlappedActor == LastHitActor)
			{
				continue;
			}

			ACharacterBase* OverlappedCharacter = Cast<ACharacterBase>(OverlappedActor);
			if (!::IsValid(OverlappedCharacter))
			{
				continue;
			}

			if (!EnumHasAnyFlags(OverlappedCharacter->ObjectType, EObjectType::Player))
			{
				continue;
			}

			if (TeamSide != OverlappedCharacter->TeamSide)
			{
				// 추가적인 거리 확인
				float DistanceToPlayer = FVector::Dist(OverlapLocation, OverlappedCharacter->GetActorLocation());
				if (DistanceToPlayer <= Distance)
				{
					PlayerCharacters.Add(OverlappedCharacter);
				}
			}
		}
	}

	// Debug drawing
	FQuat BoxRot = FQuat::Identity;
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.f;

	// 원 형태의 디버그 드로잉 (실제 거리 반경)
	DrawDebugSphere(
		GetWorld(),
		OverlapLocation,
		Distance,
		32,
		FColor::Blue,
		false,
		DebugLifeTime
	);
}
int32 AMinionBase::GetRelativeDirection(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::GetRelativeDirection] OtherActor is null."));
		return -1; // OtherActor가 유효하지 않은 경우
	}

	// 현재 캐릭터와 상대 캐릭터의 위치 벡터
	FVector MyLocation = GetActorLocation();
	FVector OtherLocation = OtherActor->GetActorLocation();

	// 상대 캐릭터 방향 벡터
	FVector DirectionToOther = (OtherLocation - MyLocation).GetSafeNormal();

	// 현재 캐릭터의 앞을 향하는 방향 벡터
	FVector ForwardVector = GetActorForwardVector();

	// Dot Product를 이용하여 방향 계산
	float ForwardDot = FVector::DotProduct(GetActorForwardVector(), DirectionToOther);
	float RightDot = FVector::DotProduct(GetActorRightVector(), DirectionToOther);

	if (ForwardDot > 0.0f && FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
	{
		return 1; // 앞
	}
	else if (ForwardDot < 0.0f && FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
	{
		return 2; // 뒤
	}
	else if (RightDot > 0.0f && FMath::Abs(RightDot) > FMath::Abs(ForwardDot))
	{
		return 3; // 오른쪽
	}
	else if (RightDot < 0.0f && FMath::Abs(RightDot) > FMath::Abs(ForwardDot))
	{
		return 4; // 왼쪽
	}

	// 기본값 반환
	return 0;
}


float AMinionBase::GetExpBounty() const
{
	return ExpBounty;
}

void AMinionBase::SetExpBounty(float NewExpBounty)
{
	ExpBounty = NewExpBounty;
}

int32 AMinionBase::GetGoldBounty() const
{
	return GoldBounty;
}

void AMinionBase::SetGoldBounty(int32 NewGoldBounty)
{
	GoldBounty = NewGoldBounty;
}

void AMinionBase::OnRep_SkeletalMesh()
{
	if (ReplicatedSkeletalMesh)
	{
		GetMesh()->SetSkeletalMesh(ReplicatedSkeletalMesh);
	}
}

void AMinionBase::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::MontageEnded] Montage is null."));
		return;
	}

	if (Montage->GetFName() == FName("Attack_Montage") && HasAuthority())
	{
		ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Attacking);
		ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::AttackEnded);
	}
	else if (Montage->GetFName() == FName("Death_Montage"))
	{
		if (USkeletalMeshComponent* MeshComponent = GetMesh())
		{
			MeshComponent->SetCollisionProfileName("Ragdoll");
			MeshComponent->SetAllBodiesSimulatePhysics(true);
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetPhysicsBlendWeight(0.0f);

			GetWorld()->GetTimerManager().SetTimer(DeathMontageTimerHandle, this, &AMinionBase::EnableRagdoll, RagdollBlendTime, false);
			GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &AMinionBase::StartFadeOut, 0.05f, true, 2.0f);
		}
	}
}

void AMinionBase::Ability_LMB()
{
	if (!::IsValid(AbilityStatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::Ability_LMB] AbilityStatComponent is not valid."));
		return;
	}

	bool bIsAbilityReady = AbilityStatComponent->IsAbilityReady(EAbilityID::Ability_LMB);
	if (!bIsAbilityReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::Ability_LMB] Ability_LMB is not ready."));
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("LMB", TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Attack_Montage.Attack_Montage"));
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::Ability_LMB] Failed to load or find montage."));
		return;
	}

	ABaseAIController* AI = Cast<ABaseAIController>(GetController());
	if (!AI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::Ability_LMB] Failed to get AIController."));
		return;
	}

	const float MontageDuration = Montage->GetSectionLength(Ability_LMB_CurrentComboCount - 1);
	const float PlayRate = SetAnimPlayRate(MontageDuration);

	PlayAnimMontage(Montage, PlayRate, FName(*FString::Printf(TEXT("Attack%d"), Ability_LMB_CurrentComboCount)));
	PlayMontage_NetMulticast(Montage, PlayRate, FName(*FString::Printf(TEXT("Attack%d"), Ability_LMB_CurrentComboCount)));

	ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::Attacking);

	AI->GetBlackboardComponent()->SetValueAsBool(ABaseAIController::IsAbilityReadyKey, false);

	AbilityStatComponent->UseAbility(EAbilityID::Ability_LMB, GetWorld()->GetTimeSeconds());
	AbilityStatComponent->StartAbilityCooldown(EAbilityID::Ability_LMB);
	
	Ability_LMB_CurrentComboCount = FMath::Clamp<int32>((Ability_LMB_CurrentComboCount % 4) + 1, 1, Ability_LMB_MaxComboCount);
}

void AMinionBase::Ability_LMB_CheckHit()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!::IsValid(AbilityStatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::Ability_LMB_CheckHit] AbilityStatComponent is not valid."));
		return;
	}

	ABaseAIController* AI = Cast<ABaseAIController>(GetController());
	if (!AI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::Ability_LMB_CheckHit] Failed to get AIController."));
		return;
	}

	ACharacterBase* Enemy = Cast<ACharacterBase>(AI->GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
	if (!::IsValid(Enemy))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::Ability_LMB_CheckHit] Enemy is not valid or not found."));
		return;
	}

	const FAbilityStatTable& AbilityStatTable = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB);

	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float BaseAttackDamage = AbilityStatTable.AttackDamage;
	const float AD_PowerScaling = AbilityStatTable.AD_PowerScaling;

	const float FinalDamage = BaseAttackDamage + Character_AttackDamage * AD_PowerScaling;

	FDamageInformation DamageInformation;
	DamageInformation.AbilityID = EAbilityID::Ability_LMB;
	DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);

	Enemy->ReceiveDamage(DamageInformation, AIController, this);
}

void AMinionBase::AttackEnded()
{
	
}