// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NonPlayerCharacter_Aurora.h"
#include "Characters/AOSCharacterBase.h"
#include "Controllers/NPCAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animations/NPCAnimInstance.h"
#include "Components/StatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ANonPlayerCharacter_Aurora::ANonPlayerCharacter_Aurora()
{
	PrimaryActorTick.bCanEverTick = false;

	CurrentComboCount = 0;
	MaxComboCount = 4;
	bIsAttacking = false;
}

void ANonPlayerCharacter_Aurora::BeginPlay()
{
	Super::BeginPlay();

	UNPCAnimInstance* NPCAnimInstance = Cast<UNPCAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(AnimInstance))
	{
		NPCAnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::MontageEnded);
		NPCAnimInstance->OnNPCCanNextCombo.BindUObject(this, &ThisClass::CanNextCombo);
	}
}

void ANonPlayerCharacter_Aurora::Ability_Q()
{
}

void ANonPlayerCharacter_Aurora::Ability_E()
{
}

void ANonPlayerCharacter_Aurora::Ability_LMB()
{

}

void ANonPlayerCharacter_Aurora::CanNextCombo()
{
	bool bResult = false;

	UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(AIController->GetBlackboardComponent());

	if (::IsValid(AIController))
	{
		if (::IsValid(BlackboardComponent))
		{
		}
	}

	if (bResult == true)
	{
		if (::IsValid(BlackboardComponent))
		{
			
		}
	}
}

void ANonPlayerCharacter_Aurora::OnPreDamageReceived(float FinalDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("ANonPlayerCharacter_Aurora::OnPreDamageReceived() FinalDamage: %f"), FinalDamage);
}

void ANonPlayerCharacter_Aurora::MontageEnded(UAnimMontage* Montage, bool bIsInterrupt)
{
	CurrentComboCount = 0;
	bIsAttacking = false;
}