// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NonPlayerCharacterBase.h"
#include "Controllers/NPCAIController.h"
#include "Components/StatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animations/NPCAnimInstance.h"
#include "UI/UW_StateBar.h"
#include "GameFramework/CharacterMovementComponent.h"

ANonPlayerCharacterBase::ANonPlayerCharacterBase()
{
	AIControllerClass = ANPCAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationYaw = false;

	float CharacterHalfHeight = 95.f;
	float CharacterRadius = 40.f;

	FVector PivotPosition(0.f, 0.f, -CharacterHalfHeight);
	FRotator PivotRotation(0.f, -90.f, 0.f);
	GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	WidgetComponent = CreateDefaultSubobject<UCharacterWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetRootComponent());
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));

	static ConstructorHelpers::FClassFinder<UUserWidgetBase> StateBarWidgetRef (TEXT("/Game/ProjectAOS/UI/WBP_StateBar.WBP_StateBar_C"));
	if (StateBarWidgetRef.Succeeded())
	{
		WidgetComponent->SetWidgetClass(StateBarWidgetRef.Class);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(200.0f, 35.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	CurrentComboCount = 0;
	MaxComboCount = 4;
	bIsAttacking = false;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));
}

void ANonPlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = Cast<UNPCAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(AnimInstance))
	{
		AnimInstance->OnNPCCanNextCombo.BindUObject(this, &ThisClass::Ability_LMB);
	}

	AIController = Cast<ANPCAIController>(GetController());
}

void ANonPlayerCharacterBase::SetWidget(UUserWidgetBase* InUserWidgetBase)
{
	UUW_StateBar* StateBar = Cast<UUW_StateBar>(InUserWidgetBase);
	
	if (::IsValid(StateBar))	
	{
		StateBar->InitializeStateBar(StatComponent);
	}
}

void ANonPlayerCharacterBase::GetCrowdControl(ECrowdControlBase InCondition, float InDuration, float InPercent)
{
	float Percent = FMath::Clamp<float>(InPercent, 0, 1);

	if (::IsValid(AIController))
	{
		UBlackboardComponent* Blackboard = Cast<UBlackboardComponent>(AIController->GetBlackboardComponent());
		if (::IsValid(Blackboard))
		{
			switch (InCondition)
			{
			case ECrowdControlBase::None:

				break;
			case ECrowdControlBase::Slow:
				LastMovementSpeed = StatComponent->GetMovementSpeed();
				ChangeMovementSpeed(0, LastMovementSpeed * (1 - Percent));

				GetWorld()->GetTimerManager().SetTimer(
					CrowdControlTimer,
					FTimerDelegate::CreateLambda([&]()
						{
							ChangeMovementSpeed(0, LastMovementSpeed);
						}
					),
					0.1f,
					false,
					InDuration
				);

				break;
			case ECrowdControlBase::Cripple:

				break;
			case ECrowdControlBase::Silence:

				break;
			case ECrowdControlBase::Blind:

				break;
			case ECrowdControlBase::BlockedSight:

				break;
			case ECrowdControlBase::Snare:

				break;
			case ECrowdControlBase::Stun:
				AnimInstance->StopAllMontages(0.1f);
				AnimInstance->PlayMontage(Stun_Montage);
				Blackboard->SetValueAsBool(AIController->IsGetCrowdControl, true);

				GetWorld()->GetTimerManager().SetTimer(
					CrowdControlTimer,
					FTimerDelegate::CreateLambda([&]()
						{
							UBlackboardComponent* Blackboard = Cast<UBlackboardComponent>(AIController->GetBlackboardComponent());
							Blackboard->SetValueAsBool(AIController->IsGetCrowdControl, false);
							AnimInstance->StopAllMontages(0.3f);
						}
					),
					0.1f,
					false,
					InDuration
				);

				break;
			case ECrowdControlBase::Taunt:

				break;
			}
		}
	}
}

const FName ANonPlayerCharacterBase::GetAttackMontageSection(const int32& Section)
{
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}