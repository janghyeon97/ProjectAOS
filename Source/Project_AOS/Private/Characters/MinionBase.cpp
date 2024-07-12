// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MinionBase.h"
#include "Controllers/MinionAIController.h"
#include "Components/MinionStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animations/MinionAnimInstance.h"
#include "UI/UW_StateBar.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

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

	float CharacterHalfHeight = 80.f;
	float CharacterRadius = 60.f;
	GetCapsuleComponent()->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);

	FVector PivotPosition(0.f, 0.f, -CharacterHalfHeight);
	FRotator PivotRotation(0.f, 0.f, 0.f);
	GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;

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

void AMinionBase::SetWidget(UUserWidgetBase* InUserWidgetBase)
{
	StateBar = Cast<UUW_StateBar>(InUserWidgetBase);

	if (::IsValid(StateBar) && ::IsValid(StatComponent))
	{
		StateBar->InitializeStateBar(StatComponent);
	}
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

void AMinionBase::SetAnimMontages(const TMap<FString, UAnimMontage*>& MontageMap)
{
	Montages = MontageMap;
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

float AMinionBase::GetExpBounty() const
{
	return ExpBounty;
}

int32 AMinionBase::GetGoldBounty() const
{
	return GoldBounty;
}

void AMinionBase::SetExpBounty(float NewExpBounty)
{
	ExpBounty = NewExpBounty;
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
	if (ForwardDot > 0.707f)
	{
		return 1; // 앞
	}
	else if (ForwardDot < -0.707f)
	{
		return 2; // 뒤
	}
	else if (RightDot > 0.707f)
	{
		return 3; // 오른쪽
	}
	else if (RightDot < -0.707f)
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

	// Overlap detection
	TArray<FOverlapResult> OutHits;
	FVector CollisionBoxSize = FVector(500.0f, 500.0f, 150.0f);
	FVector OverlapLocation = LastHitActor->GetActorLocation();
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
		UE_LOG(LogTemp, Log, TEXT("[AMinionBase::OnCharacterDeath] Overlap detection succeeded, processing hits."));
		for (const auto& OutHit : OutHits)
		{
			if (::IsValid(OutHit.GetActor()))
			{
				UE_LOG(LogTemp, Log, TEXT("[AMinionBase::OnCharacterDeath] Overlapped with actor: %s"), *OutHit.GetActor()->GetName());
				UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s"), *OutHit.GetActor()->GetName()));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] Overlap detection failed."));
	}

	// Debug drawing
	FQuat BoxRot = FQuat::Identity;
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.f;

	DrawDebugBox(
		GetWorld(),
		OverlapLocation,
		CollisionBoxSize,
		BoxRot,
		DrawColor,
		false,
		DebugLifeTime
	);
}

void AMinionBase::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage->GetName().Equals("Death_Montage"))
	{
		EnumAddFlags(CharacterState, EBaseCharacterState::Dead);

		USkeletalMeshComponent* MeshComponent = GetMesh();
		if (MeshComponent)
		{
			MeshComponent->SetAllBodiesSimulatePhysics(true);
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));

			// Physics Blend를 사용하여 전환
			MeshComponent->SetPhysicsBlendWeight(0.0f);

			GetWorld()->GetTimerManager().SetTimer(DeathMontageTimerHandle, this, &AMinionBase::EnableRagdoll, RagdollBlendTime, false);
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
	}

	// 캐릭터의 움직임 비활성화
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
		MovementComponent->StopMovementImmediately();
	}
}