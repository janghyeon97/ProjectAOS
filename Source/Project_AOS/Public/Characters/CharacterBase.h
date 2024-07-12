// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Structs/EnumTeamSide.h"
#include "Structs/EnumCrowdControl.h"
#include "Structs/EnumCharacterType.h"
#include "CharacterBase.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceiveDamageEnteredDelegate, bool&, bResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreReceiveDamageDelegate, FDamageInfomation&, DamageInfomation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostReceiveDamageDelegate, float&, FinalDamage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreDamageCalculationDelegate, FDamageInfomation&, DamageInfomation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostDamageCalculationDelegate, float&, FinalDamage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitEventTriggeredDelegate, FDamageInfomation&, DamageInfomation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackEventTriggeredDelegate, FDamageInfomation&, DamageInfomation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEffectsEventTriggeredDelegate, FDamageInfomation&, DamageInfomation);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBaseCharacterState : uint32
{
	None = 0x00,
	Dead = 0x01 << 0,
	Move = 0x01 << 1,	
	Jump = 0x01 << 2,	

	Ability_Q = 0x01 << 10,	
	Ability_E = 0x01 << 11,	
	Ability_R = 0x01 << 12,	
	Ability_LMB = 0x01 << 13,	
	Ability_RMB = 0x01 << 14,	
	AbilityUsed = 0x01 << 15,	
	SwitchAction = 0x01 << 16,	

	Basic = Move | Jump | SwitchAction,
};
ENUM_CLASS_FLAGS(EBaseCharacterState);

UCLASS()
class PROJECT_AOS_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

	// Override functions
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Delegate functions
	UFUNCTION()
	virtual void OnPreCalculateDamage(float& AttackDamage, float& AbilityPower) {};
	UFUNCTION()
	virtual void OnPreDamageReceived(float FinalDamage) {};

	UFUNCTION()
	virtual void OnRep_CharacterStateChanged();

	// Custom functions
	UFUNCTION()
	virtual void SetWidget(class UUserWidgetBase* InUserWidgetBase) {};
	UFUNCTION()
	virtual bool ValidateHit(EAbilityID AbilityID);
	UFUNCTION()
	virtual bool ReceiveDamage(FDamageInfomation DamageInfomation, AController* EventInstigator, AActor* DamageCauser);
	UFUNCTION()
	virtual void GetCrowdControl(ECrowdControlBase InCondition = ECrowdControlBase::None, float InDuration = 0.f, float InPercent = 0.f) {};
	UFUNCTION()
	virtual void ChangeMovementSpeed(float InOldMS, float InNewMS);

	// Getter functions
	class UStatComponent* GetStatComponent() const { return StatComponent; }
	class UAbilityStatComponent* GetAbilityStatComponent() const { return AbilityStatComponent; }

public:
	// Delegates
	FOnReceiveDamageEnteredDelegate OnReceiveDamageEnteredEvent;
	FOnPreDamageCalculationDelegate OnPreDamageCalculationEvent;
	FOnPostDamageCalculationDelegate OnPostDamageCalculationEvent;
	FOnPreReceiveDamageDelegate OnPreReceiveDamageEvent;
	FOnPostReceiveDamageDelegate OnPostReceiveDamageEvent;
	FOnHitEventTriggeredDelegate OnHitEventTriggered;
	FOnAttackEventTriggeredDelegate OnAttackEventTriggered;
	FOnAbilityEffectsEventTriggeredDelegate OnAbilityEffectsEventTriggered;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Character|TeamSide")
	ETeamSideBase TeamSide = ETeamSideBase::Type;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	EObjectType ObjectType = EObjectType::None;

	// 캐릭터 상태
	UPROPERTY(ReplicatedUsing = OnRep_CharacterStateChanged, Transient, VisibleAnywhere, Category = "Character", Meta = (AllowPrivateAccess))
	EBaseCharacterState CharacterState;

	EBaseCharacterState PreviousCharacterState = CharacterState;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (AllowPrivateAccess))
	TObjectPtr<class UStatComponent> StatComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AbilityStat", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAbilityStatComponent> AbilityStatComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AbilityStat", Meta = (AllowPrivateAccess))
	TObjectPtr<class AActor> LastHitActor;

	float LastMovementSpeed;
	float LastAttackSpeed;
};
