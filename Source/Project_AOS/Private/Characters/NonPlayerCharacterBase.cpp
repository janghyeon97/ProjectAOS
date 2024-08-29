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

	UNPCAnimInstance* NPCAnimInstance = Cast<UNPCAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(NPCAnimInstance))
	{
		NPCAnimInstance->OnNPCCanNextCombo.BindUObject(this, &ThisClass::Ability_LMB);
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

const FName ANonPlayerCharacterBase::GetAttackMontageSection(const int32& Section)
{
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}