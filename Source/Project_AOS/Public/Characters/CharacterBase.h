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
	None			= 0x00		UMETA(Hidden),
	Death			= 0x01 << 0 UMETA(DisplayName = "Death"),
	Move			= 0x01 << 1 UMETA(DisplayName = "Move"),
	Jump			= 0x01 << 2 UMETA(DisplayName = "Jump"),

	Ability_Q		= 0x01 << 3 UMETA(DisplayName = "Ability_Q"),
	Ability_E		= 0x01 << 4 UMETA(DisplayName = "Ability_E"),
	Ability_R		= 0x01 << 5 UMETA(DisplayName = "Ability_R"),
	Ability_LMB		= 0x01 << 6 UMETA(DisplayName = "Ability_LMB"),
	Ability_RMB		= 0x01 << 7 UMETA(DisplayName = "Ability_RMB"),
	AbilityUsed		= 0x01 << 8 UMETA(DisplayName = "AbilityUsed"),
	SwitchAction	= 0x01 << 9 UMETA(DisplayName = "SwitchAction"),
};
ENUM_CLASS_FLAGS(EBaseCharacterState);

UENUM(BlueprintType)
enum class ECharacterStateOperation : uint8
{
	Add,
	Remove
};

UCLASS()
class PROJECT_AOS_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

protected:
	// Override functions
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void LogCharacterState(EBaseCharacterState State, const FString& Context);
	bool IsValidCharacterState(EBaseCharacterState State);
	bool IsValidCombinedCharacterState(EBaseCharacterState State);

public:
	uint32 GetCharacterState() const { return ReplicatedCharacterState; };
	void SetCharacterState(uint32 NewState);

	UFUNCTION(Client, Reliable)
	void UpdateCharacterState(uint32 NewState);

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
	virtual void GetCrowdControl(EBaseCrowdControl InCondition = EBaseCrowdControl::None, float InDuration = 0.f, float InPercent = 0.f) {};

	UFUNCTION()
	virtual void ChangeMovementSpeed(float InOldMS, float InNewMS);

	UFUNCTION(Server, Reliable)
	virtual void ModifyCharacterState(ECharacterStateOperation Operation, EBaseCharacterState StateFlag);

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
	UPROPERTY(Replicated, Transient, VisibleAnywhere, Category = "CharacterState")
	uint32 ReplicatedCharacterState;

	UPROPERTY(Transient, VisibleAnywhere, Category = "CharacterState")
	EBaseCharacterState CharacterState;

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
