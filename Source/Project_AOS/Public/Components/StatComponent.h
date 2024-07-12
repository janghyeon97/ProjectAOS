// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"

#pragma region DefineDelegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentHPChangedDelegate, float, InOldCurrentHP, float, InNewCurrentHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHPChangedDelegate, float, InOldMaxHP, float, InNewMaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOutOfCurrentHPDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHPEqualsMaxHPDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHPNotEqualsMaxHPDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentMPChangedDelegate, float, InOldCurrentMP, float, InNewCurrentMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxMPChangedDelegate, float, InOldMaxMP, float, InNewMaxMP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMPEqualsMaxMPDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMPNotEqualsMaxMPDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentLevelChangedDelegate, int32, InOldCurrentLevel, int32, InNewCurrentLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentEXPChangedDelegate, float, InOldCurrentEXP, float, InNewCurrentEXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxEXPChangedDelegate, float, InOldMaxEXP, float, InNewMaxEXP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPRegenChangedDelegate, float, InOldHPRegen, float, InNewHPRegen);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMPRegenChangedDelegate, float, InOldMPRegen, float, InNewMPRegen);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackDamageChangedDelegate, float, InOldAttackDamage, float, InNewAttackDamage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityPowerChangedDelegate, float, InOldAbilityPower, float, InNewAbilityPower);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDefensePowerChangedDelegate, float, InOldDefensePower, float, InNewDefensePower);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagicResistanceChangedDelegate, float, InOldMagicResistance, float, InNewMagicResistance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackSpeedChangedDelegate, float, InOldAttackSpeed, float, InNewAttackSpeed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityHasteChangedDelegate, int32, InOldAbilityHaste, int32, InNewAbilityHaste);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCriticalChanceChangedDelegate, int32, InOldCriticalChance, int32, InNewCriticalChance);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMovementSpeedChangedDelegate, float, InOldMovementSpeed, float, InNewMovementSpeed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterStatReplicatedDelegate);
#pragma endregion

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_AOS_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStatComponent();

	void ForceReplication();

	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
	virtual void InitializeStatComponent(const int32 InChampionIndex);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

public:
	UFUNCTION(Server, Reliable)
	void OnHUDInitializationCompleted_Server();

#pragma region GetterAndSetter
	// Getter functions
	float GetMaxHP() const { return MaxHP; }
	float GetCurrentHP() const { return CurrentHP; }
	float GetMaxMP() const { return MaxMP; }
	float GetCurrentMP() const { return CurrentMP; }
	float GetMaxEXP() const { return MaxEXP; }
	float GetCurrentEXP() const { return CurrentEXP; }
	int32 GetCurrentLevel() const { return CurrentLevel; }
	float GetHealthRegeneration() const { return HealthRegeneration; }
	float GetManaRegeneration() const { return ManaRegeneration; }
	float GetAttackDamage() const { return AttackDamage; }
	float GetAbilityPower() const { return AbilityPower; }
	float GetDefensePower() const { return DefensePower; }
	float GetMagicResistance() const { return MagicResistance; }
	int32 GetAbilityHaste() const { return AbilityHaste; }
	float GetAttackSpeed() const { return AttackSpeed; }
	int32 GetCriticalChance() const { return CriticalChance; }
	float GetMovementSpeed() const { return MovementSpeed; }

	// Setter functions
	virtual void SetMaxHP(float InMaxHP);
	virtual void SetCurrentHP(float InCurrentHP);
	virtual void SetMaxMP(float InMaxMP);
	virtual void SetCurrentMP(float InCurrentMP);
	virtual void SetMaxEXP(float InMaxEXP);
	virtual void SetCurrentEXP(float InCurrentEXP);
	virtual void SetCurrentLevel(int32 InCurrentLevel);
	virtual void SetHealthRegeneration(float InHealthRegeneration);
	virtual void SetManaRegeneration(float InManaRegeneration);
	virtual void SetAttackDamage(float InAttackDamage);
	virtual void SetAbilityPower(float InAbilityPower);
	virtual void SetDefensePower(float InDefensePower);
	virtual void SetMagicResistance(float InMagicResistance);
	virtual void SetAbilityHaste(int32 InAbilityHaste);
	virtual void SetAttackSpeed(float InAttackSpeed);
	virtual void SetCriticalChance(int32 InCriticalChance);
	virtual void SetMovementSpeed(float InMovementSpeed);
#pragma endregion

public:
	FOnCurrentHPChangedDelegate OnCurrentHPChanged;
	FOnMaxHPChangedDelegate OnMaxHPChanged;
	FOnOutOfCurrentHPDelegate OnOutOfCurrentHP;
	FOnHPEqualsMaxHPDelegate OnHPEqualsMaxHP;
	FOnHPNotEqualsMaxHPDelegate OnHPNotEqualsMaxHP;

	FOnCurrentMPChangedDelegate OnCurrentMPChanged;
	FOnMaxMPChangedDelegate OnMaxMPChanged;
	FOnMPEqualsMaxMPDelegate OnMPEqualsMaxMP;
	FOnMPNotEqualsMaxMPDelegate OnMPNotEqualsMaxMP;

	FOnHPRegenChangedDelegate OnHPRegenChanged;
	FOnMPRegenChangedDelegate OnMPRegenChanged;

	FOnCurrentLevelChangedDelegate OnCurrentLevelChanged;
	FOnCurrentEXPChangedDelegate OnCurrentEXPChanged;
	FOnMaxEXPChangedDelegate OnMaxEXPChanged;

	FOnAttackDamageChangedDelegate OnAttackDamageChanged;
	FOnAbilityPowerChangedDelegate OnAbilityPowerChanged;
	FOnDefensePowerChangedDelegate OnDefensePowerChanged;
	FOnMagicResistanceChangedDelegate OnMagicResistanceChanged;
	FOnAttackSpeedChangedDelegate OnAttackSpeedChanged;
	FOnAbilityHasteChangedDelegate OnAbilityHasteChanged;
	FOnCriticalChanceChangedDelegate OnCriticalChanceChanged;
	FOnMovementSpeedChangedDelegate OnMovementSpeedChanged;

	FOnCharacterStatReplicatedDelegate OnCharacterStatReplicated;

private:
	UFUNCTION()
	void OnRep_CharacterStatReplicated();

	UFUNCTION(NetMulticast, Reliable)
	void OnOutOfCurrentHP_NetMulticast();

	UFUNCTION(NetMulticast, Reliable)
	void OnMaxHPChanged_NetMulticast(float InOldMaxHP, float InNewMaxHP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentHPChanged_NetMulticast(float InOldCurrentHP, float InNewCurrentHP);

	UFUNCTION(NetMulticast, Reliable)
	void OnMaxMPChanged_NetMulticast(float InOldMaxMP, float InNewMaxMP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentMPChanged_NetMulticast(float InOldCurrentMP, float InNewCurrentMP);

	UFUNCTION(NetMulticast, Reliable)
	void OnMaxEXPChanged_NetMulticast(float InOldMaxEXP, float InNewMaxEXP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentEXPChanged_NetMulticast(float InOldCurrentEXP, float InNewCurrentEXP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentLevelChanged_NetMulticast(int32 InOldCurrentLevel, int32 InNewCurrentLevel);

	UFUNCTION(NetMulticast, Reliable)
	void OnHealthRegenerationChanged_NetMulticast(float InOldHealthRegeneration, float InNewHealthRegeneration);

	UFUNCTION(NetMulticast, Reliable)
	void OnManaRegenerationChanged_NetMulticast(float InOldManaRegeneration, float InNewManaRegeneration);

	UFUNCTION(NetMulticast, Reliable)
	void OnAttackDamageChanged_NetMulticast(float InOldAttackDamage, float InNewAttackDamage);

	UFUNCTION(NetMulticast, Reliable)
	void OnAbilityPowerChanged_NetMulticast(float InOldAbilityPower, float InNewAbilityPower);

	UFUNCTION(NetMulticast, Reliable)
	void OnDefensePowerChanged_NetMulticast(float InOldDefensePower, float InNewDefensePower);

	UFUNCTION(NetMulticast, Reliable)
	void OnMagicResistanceChanged_NetMulticast(float InOldMagicResistance, float InNewMagicResistance);

	UFUNCTION(NetMulticast, Reliable)
	void OnAttackSpeedChanged_NetMulticast(float InOldAttackSpeed, float InNewAttackSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void OnAbilityHasteChanged_NetMulticast(int32 InOldAbilityHaste, int32 InNewAbilityHaste);

	UFUNCTION(NetMulticast, Reliable)
	void OnCriticalChanceChanged_NetMulticast(int32 InOldCriticalChance, int32 InNewCriticalChance);

	UFUNCTION(NetMulticast, Reliable)
	void OnMovementSpeedChanged_NetMulticast(float InOldMovementSpeed, float InNewMovementSpeed);

private:	
	UPROPERTY()
	TObjectPtr<class UAOSGameInstance> GameInstance;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterStatReplicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float MaxHP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float CurrentHP;

	UPROPERTY(Replicated, Transient,VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float MaxMP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float CurrentMP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float MaxEXP;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float CurrentEXP;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	int32 MaxLevel;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	int32 CurrentLevel;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float HealthRegeneration;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float ManaRegeneration;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float AttackDamage;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float AbilityPower;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float DefensePower;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float MagicResistance;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float AttackSpeed;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	int32 AbilityHaste;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	int32 CriticalChance;

	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|Stat", Meta = (AllowPrivateAccess))
	float MovementSpeed;

	int32 ChampionIndex = 0;
};
