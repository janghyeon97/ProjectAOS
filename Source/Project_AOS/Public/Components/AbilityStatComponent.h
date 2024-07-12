// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityQCooldownChangedDelegate, float, MaxCooldown, float, CurrentCooldown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityECooldownChangedDelegate, float, MaxCooldown, float, CurrentCooldown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityRCooldownChangedDelegate, float, MaxCooldown, float, CurrentCooldown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityLMBCooldownChangedDelegate, float, MaxCooldown, float, CurrentCooldown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityRMBCooldownChangedDelegate, float, MaxCooldown, float, CurrentCooldown);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAbilityVisibilityChangedDelegate, enum EAbilityID AbilityID, bool Visibility);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpgradeWidgetVisibilityChangedDelegate, enum EAbilityID AbilityID, bool Visibility);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAbilityLevelChangedDelegate, enum EAbilityID AbilityID, int Level);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisibleDescriptionDelegate, FName, String);

UENUM(BlueprintType)
enum class EAbilityTimerStart : uint8
{
	None = 0x00		UMETA(Hidden),
	OnAbilityStart = 0x01 << 0 UMETA(DisplayName = "On Ability Start"),
	OnMaxUses = 0x01 << 2 UMETA(DisplayName = "On Max Uses")
};

USTRUCT(BlueprintType)
struct FAbilityAttributes
{
	GENERATED_BODY()

public:
	FAbilityAttributes()
		: Name(FString())
		, Description(FString())
		, MaxLevel(0)
		, CurrentLevel(0)
		, MaxInstances(0)
		, InstanceIndex(0)
		, ReuseDuration(0.f)
		, bAblityReady(false)
		, bIsUpgradable(false)
	{
	}

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	FString Name;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	FString Description;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	int32 MaxLevel;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	int32 CurrentLevel;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	int32 MaxInstances;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	int32 InstanceIndex;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float ReuseDuration;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	bool bAblityReady;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	bool bIsUpgradable;
};

USTRUCT(BlueprintType)
struct FAbilityStatTable
{
	GENERATED_BODY()

public:
	FAbilityStatTable()
		: CurrentLevel(0)
		, InstanceIndex(0)
		, Ability_AttackDamage(0.f)
		, Ability_AbilityPower(0.f)
		, Ability_AD_Ratio(0.f)
		, Ability_AP_Ratio(0.f)
		, Cooldown(0.f)
		, Cost(0.f)
		, ReuseDuration(0.f)
		, UniqueValueName(TArray<FString>())
		, UniqueValue(TArray<float>())
	{
	}

	bool operator==(const FAbilityStatTable& Other) const
	{
		return Name == Other.Name;
	}

	bool IsValid() const
	{
		return !Name.IsEmpty();
	}

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	FString Name;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	int32 CurrentLevel;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	int32 InstanceIndex;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float Ability_AttackDamage;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float Ability_AbilityPower;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float Ability_AD_Ratio;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float Ability_AP_Ratio;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float Cooldown;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float Cost;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float ReuseDuration;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	TArray<FString> UniqueValueName;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	TArray<float> UniqueValue;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_AOS_API UAbilityStatComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class AAOSCharacterBase;

public:
	UAbilityStatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void InitializeAbility(EAbilityID AbilityID, const int32 InLevel);
	void InitAbilityStatComponent(class UStatComponent* InStatComponent, const int32 CharacterIndex);

	FAbilityAttributes& GetAbilityInfomation(EAbilityID AbilityID);
	TArray<FAbilityStatTable>& GetAbilityStatTable(EAbilityID AbilityID);
	FAbilityStatTable& GetAbilityStatTable(EAbilityID AbilityID, int32 InstanceIndex);
	bool IsAbilityReady(EAbilityID AbilityID);
	void BanUseAbilityFewSeconds(float Seconds);

	UFUNCTION(Server, Reliable)
	void UseAbility(EAbilityID AbilityID, float CurrentTime);

	UFUNCTION(Server, Reliable)
	void StartAbilityCooldown(EAbilityID AbilityID);

	UFUNCTION(Client, Reliable)
	void Ability_Q_CooldownChanged_Client(const float InAbility_Q_MaxCooldown, const float InAbility_Q_Cooldown);

	UFUNCTION(Client, Reliable)
	void Ability_E_CooldownChanged_Client(const float InAbility_Q_MaxCooldown, const float InAbility_Q_Cooldown);

	UFUNCTION(Client, Reliable)
	void Ability_R_CooldownChanged_Client(const float InAbility_Q_MaxCooldown, const float InAbility_Q_Cooldown);

	UFUNCTION(Client, Reliable)
	void Ability_LMB_CooldownChanged_Client(const float InAbility_Q_MaxCooldown, const float InAbility_Q_Cooldown);

	UFUNCTION(Client, Reliable)
	void Ability_RMB_CooldownChanged_Client(const float InAbility_Q_MaxCooldown, const float InAbility_Q_Cooldown);

	UFUNCTION(Server, Reliable)
	void OnHUDInitializationCompleted_Server();

	UFUNCTION(Server, Reliable)
	void ToggleLevelUpUI_Server(bool Visibility);

	UFUNCTION(Server, Reliable)
	void UpdateLevelUpUI_Server(int32 InOldCurrentLevel, int32 InNewCurrentLevel);

	UFUNCTION(Client, Reliable)
	void BroadcastUpgradeWidgetVisibility_Client(EAbilityID AbilityID, bool InVisibility);

	UFUNCTION(Client, Reliable)
	void BroadcastAbilityVisibility_Client(EAbilityID AbilityID, bool InVisibility);

	UFUNCTION(Client, Reliable)
	void BroadcastAbilityLevelChanged_Client(EAbilityID AbilityID, int InLevel);

public:
	FOnAbilityQCooldownChangedDelegate OnAbilityQCooldownChanged;
	FOnAbilityECooldownChangedDelegate OnAbilityECooldownChanged;
	FOnAbilityRCooldownChangedDelegate OnAbilityRCooldownChanged;
	FOnAbilityLMBCooldownChangedDelegate OnAbilityLMBCooldownChanged;
	FOnAbilityRMBCooldownChangedDelegate OnAbilityRMBCooldownChanged;
	FOnVisibleDescriptionDelegate OnVisibleDescription;
	FOnAbilityLevelChangedDelegate OnAbilityLevelChanged;
	FOnAbilityVisibilityChangedDelegate OnAbilityVisibilityChanged;
	FOnUpgradeWidgetVisibilityChangedDelegate OnUpgradeWidgetVisibilityChanged;

private:
	void InitializeAbility(EAbilityID AbilityID, FAbilityAttributes& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, const int32 InLevel);
	void InitializeAbilityInfomation(EAbilityID AbilityID, FAbilityAttributes& AbilityInfo, int32 CharacterIndex);

	void UseSpecificAbility(FAbilityAttributes& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, float& Cooldown, float& LastUseTime, float CurrentTime, EAbilityID AbilityID);
	void UpdateAbilityUpgradeStatus(EAbilityID AbilityID, FAbilityAttributes& AbilityInfo, int32 InNewCurrentLevel);

	UPROPERTY()
	TObjectPtr<class UAOSGameInstance> GameInstance;

	UPROPERTY()
	TObjectPtr<class AAOSCharacterBase> OwnerCharacter;

	UPROPERTY()
	TWeakObjectPtr<class UStatComponent> StatComponent;

	UPROPERTY(Replicated)
	FAbilityAttributes Ability_Q_Info;

	UPROPERTY(Replicated)
	FAbilityAttributes Ability_E_Info;

	UPROPERTY(Replicated)
	FAbilityAttributes Ability_R_Info;

	UPROPERTY(Replicated)
	FAbilityAttributes Ability_LMB_Info;

	UPROPERTY(Replicated)
	FAbilityAttributes Ability_RMB_Info;

	UPROPERTY(Replicated)
	TArray<FAbilityStatTable> Ability_Q_Stat;

	UPROPERTY(Replicated)
	TArray<FAbilityStatTable> Ability_E_Stat;

	UPROPERTY(Replicated)
	TArray<FAbilityStatTable> Ability_R_Stat;

	UPROPERTY(Replicated)
	TArray<FAbilityStatTable> Ability_LMB_Stat;

	UPROPERTY(Replicated)
	TArray<FAbilityStatTable> Ability_RMB_Stat;

	float Ability_Q_LastUseTime;
	float Ability_E_LastUseTime;
	float Ability_R_LastUseTime;
	float Ability_LMB_LastUseTime;
	float Ability_RMB_LastUseTime;

	float Ability_Q_MaxCooldown;
	float Ability_E_MaxCooldown;
	float Ability_R_MaxCooldown;
	float Ability_LMB_MaxCooldown;
	float Ability_RMB_MaxCooldown;

	float Ability_Q_Cooldown;
	float Ability_E_Cooldown;
	float Ability_R_Cooldown;
	float Ability_LMB_Cooldown;
	float Ability_RMB_Cooldown;

	FTimerHandle Ability_Ban_Timer;
	FTimerHandle Ability_Q_Timer;
	FTimerHandle Ability_E_Timer;
	FTimerHandle Ability_R_Timer;
	FTimerHandle Ability_LMB_Timer;
	FTimerHandle Ability_RMB_Timer;

	int32 ChampionIndex = 0;
};