// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/MinionBase.h"
#include "Controllers/MinionAIController.h"
#include "Components/MinionStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/AOSGameMode.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animations/MinionAnimInstance.h"
#include "UI/UW_StateBar.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

AMinionBase::AMinionBase()
{
	AIControllerClass = AMinionAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bReplicates = true;
	SetReplicateMovement(true);

	StatComponent = CreateDefaultSubobject<UMinionStatComponent>(TEXT("MinionStatComponent"));

	if (HasAuthority())
	{
		StatComponent->SetIsReplicated(true);
	}

	WidgetComponent = CreateDefaultSubobject<UCharacterWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetRootComponent());
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));

	static ConstructorHelpers::FClassFinder<UUserWidgetBase> StateBarWidgetRef(TEXT("/Game/ProjectAOS/UI/WBP_StateBar.WBP_StateBar_C"));
	if (StateBarWidgetRef.Succeeded())
	{
		WidgetComponent->SetWidgetClass(StateBarWidgetRef.Class);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(200.0f, 35.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	float CharacterHalfHeight = 65.f;
	float CharacterRadius = 60.f;
	GetCapsuleComponent()->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);

	FVector PivotPosition(0.f, 0.f, -60.f);
	FRotator PivotRotation(0.f, 0.f, -90.f);
	GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;

	ObjectType = EObjectType::Minion;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Minion"));
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
	}

	AnimInstance = Cast<UMinionAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::MontageEnded);
	}

	AIController = Cast<AMinionAIController>(GetController());
}

void AMinionBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		if (StatComponent)
		{
			UMinionStatComponent* MinionStatComponent = Cast<UMinionStatComponent>(StatComponent);
			if (MinionStatComponent)
			{
				MinionStatComponent->InitializeStatComponent(MinionType);
			}
		}
	}
}

void AMinionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMinionBase, ReplicatedSkeletalMesh);
}

void AMinionBase::SetWidget(UUserWidgetBase* InUserWidgetBase)
{
	StateBar = Cast<UUW_StateBar>(InUserWidgetBase);

	if (::IsValid(StateBar) && ::IsValid(StatComponent))
	{
		StateBar->InitializeStateBar(StatComponent);
	}
}

void AMinionBase::SetAnimMontages(const TMap<FString, UAnimMontage*>& MontageMap)
{
	Montages = MontageMap;
}

void AMinionBase::PlayMontage_Server_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayMontage_NetMulticast(Montage, PlayRate);
}

void AMinionBase::PlayMontage_NetMulticast_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->PlayMontage(Montage, PlayRate);
}

void AMinionBase::StopAllMontages_Server_Implementation(float BlendOut)
{
	StopAllMontages_NetMulticast(BlendOut);
}

void AMinionBase::StopAllMontages_NetMulticast_Implementation(float BlendOut)
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->StopAllMontages(BlendOut);
}

void AMinionBase::MontageJumpToSection_Server_Implementation(UAnimMontage* Montage, FName SectionName, float PlayRate)
{
	MontageJumpToSection_NetMulticast(Montage, SectionName, PlayRate);
}

void AMinionBase::MontageJumpToSection_NetMulticast_Implementation(UAnimMontage* Montage, FName SectionName, float PlayRate)
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->Montage_SetPlayRate(Montage, PlayRate);
	AnimInstance->Montage_JumpToSection(SectionName, Montage);
}

void AMinionBase::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage->GetName().Equals("Death_Montage"))
	{
		EnumAddFlags(CharacterState, EBaseCharacterState::Death);

		UCapsuleComponent* ValidCapsuleComponent = GetCapsuleComponent();
		if (ValidCapsuleComponent)
		{
			ValidCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		USkeletalMeshComponent* MeshComponent = GetMesh();
		if (MeshComponent)
		{
			MeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			MeshComponent->SetAllBodiesSimulatePhysics(true);
			MeshComponent->SetSimulatePhysics(true);

			// Physics Blend를 사용하여 전환
			MeshComponent->SetPhysicsBlendWeight(0.0f);

			GetWorld()->GetTimerManager().SetTimer(DeathMontageTimerHandle, this, &AMinionBase::EnableRagdoll, RagdollBlendTime, false);
			GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &AMinionBase::StartFadeOut, 0.05f, true, 2.0f);
			UE_LOG(LogTemp, Log, TEXT("[AMyCharacter::StartRagdollTransition] Ragdoll transition started."));
		}
	}
}

void AMinionBase::EnableRagdoll()
{
	// 메쉬의 물리적 상태를 Ragdoll로 전환
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetPhysicsBlendWeight(1.0f);
		MeshComponent->WakeAllRigidBodies();
		ApplyDirectionalImpulse();
	}

	// 캐릭터의 움직임 비활성화
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
		MovementComponent->StopMovementImmediately();
	}
}

void AMinionBase::StartFadeOut()
{
	CurrentFadeDeath += (0.05f / FadeOutDuration);
	if (CurrentFadeDeath >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		CurrentFadeDeath = 1.0f;
		Destroy(true, true);
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

void AMinionBase::OnCharacterDeath()
{
	if (!LastHitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] LastHitActor is null."));
		return;
	}

	int32 RelativeDirection = GetRelativeDirection(LastHitActor);
	if (RelativeDirection == -1 || RelativeDirection == 0)
	{
		RelativeDirection = 1;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] RelativeDirection %d"), RelativeDirection);

	UAnimMontage** DeathMontage = Montages.Find("Death");
	if (!DeathMontage || !::IsValid(*DeathMontage))
	{
		*DeathMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), NULL, TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/HitReact_Montage.HitReact_Montage")));
		if (!(*DeathMontage))
		{
			return;
		}
	}

	StatComponent->OnOutOfCurrentHP.RemoveDynamic(this, &ThisClass::OnCharacterDeath);

	// Disable character collision and movement
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	// Disable actor
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// Play death animation
	FName SectionName = FName(*FString::Printf(TEXT("Death%d"), RelativeDirection));
	PlayMontage_Server(*DeathMontage, 1.0f);
	MontageJumpToSection_Server(*DeathMontage, SectionName, 1.0f);

	AAOSGameMode* GM = Cast<AAOSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
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

void AMinionBase::ApplyDirectionalImpulse()
{
	if (!LastHitActor) return;

	FVector ImpulseDirection = FVector::ZeroVector;

	int32 RelativeDirection = GetRelativeDirection(LastHitActor);

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
		ImpulseDirection = GetActorRightVector();
		break;
	case 4: // 왼쪽
		ImpulseDirection = -GetActorRightVector();
		break;
	default:
		break;
	}

	// 임펄스 적용
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		FVector Impulse = ImpulseDirection * ImpulseStrength;
		MeshComponent->AddImpulse(Impulse, NAME_None, true);
		UE_LOG(LogTemp, Log, TEXT("[AMinionBase::ApplyDirectionalImpulse] Applied impulse: %s"), *Impulse.ToString());
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

		if (Eliminator)
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

int32 AMinionBase::GetRelativeDirection(AActor* OtherActor) const
{
	if (!OtherActor) return -1; // OtherActor가 유효하지 않은 경우

	// 현재 캐릭터와 상대 캐릭터의 위치 벡터
	FVector MyLocation = GetActorLocation();
	FVector OtherLocation = OtherActor->GetActorLocation();

	// 상대 캐릭터 방향 벡터
	FVector DirectionToOther = (OtherLocation - MyLocation).GetSafeNormal();

	// 현재 캐릭터의 앞을 향하는 방향 벡터
	FVector ForwardVector = GetActorForwardVector();

	// Dot Product를 이용하여 방향 계산
	float ForwardDot = FVector::DotProduct(ForwardVector, DirectionToOther);
	float RightDot = FVector::DotProduct(GetActorRightVector(), DirectionToOther);

	// 방향에 따라 1~4 정수 반환
	if (ForwardDot > 0.0f)
	{
		return 1; // 앞
	}
	else if (ForwardDot < 0.0f)
	{
		return 2; // 뒤
	}
	else if (RightDot > 0.0f)
	{
		return 3; // 오른쪽
	}
	else if (RightDot < 0.0f)
	{
		return 4; // 왼쪽
	}

	return 0;
}

void AMinionBase::OnRep_SkeletalMesh()
{
	if (ReplicatedSkeletalMesh)
	{
		GetMesh()->SetSkeletalMesh(ReplicatedSkeletalMesh);
	}
}
