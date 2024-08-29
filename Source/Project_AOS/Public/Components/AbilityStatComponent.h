// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/AbilityData.h"
#include "Structs/EnumAbilityType.h"
#include "Structs/EnumAbilityID.h"
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


struct FStatTableRow;
struct FAbilityStatTableRow;
class ICharacterDataProviderInterface;
class UStatComponent;


USTRUCT(BlueprintType)
struct FAbilityDetails
{
	GENERATED_BODY()

public:
	FAbilityDetails()
		: AbilityID(EAbilityID::None)
		, Name(FString())
		, Description(FString())
		, MaxLevel(0)
		, CurrentLevel(0)
		, MaxInstances(0)
		, InstanceIndex(0)
		, ReuseDuration(0.f)
		, bAbilityReady(false)
		, bCanCastAbility(true)
		, bIsUpgradable(false)
		, LastUseTime(0.f)
		, MaxCooldown(0.f)
		, Cooldown(0.f)
		, AbilityType(EAbilityType::None)
		, AbilityDetection(EAbilityDetection::None)
	{
	}

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	EAbilityID AbilityID;

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
	bool bAbilityReady;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	bool bCanCastAbility;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	bool bIsUpgradable;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float LastUseTime;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float MaxCooldown;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	float Cooldown;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	EAbilityType AbilityType;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Charater|AbilityStat")
	EAbilityDetection AbilityDetection;
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
	void InitAbilityStatComponent(ICharacterDataProviderInterface* InDataProvider, UStatComponent* InStatComponent, const FName& InRowName);

	FAbilityDetails& GetAbilityInfomation(EAbilityID AbilityID);
	const FAbilityStatTable& GetAbilityStatTable(EAbilityID AbilityID) const;
	float GetUniqueValue(EAbilityID AbilityID, const FString& InKey, float DefaultValue);
	bool IsAbilityReady(EAbilityID AbilityID) const;
	void BanUseAbilityFewSeconds(float Seconds);
	void BanUseAbilityFewSeconds(EAbilityDetection AbilityDetection, float Seconds);

	float GetAbilityCurrentCooldown() const;

	TArray<FAbilityDetails*> GetAbilityDetailsPtrs();

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
	void InitializeAbility(EAbilityID AbilityID, FAbilityDetails& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, const int32 InLevel);
	void InitializeAbilityInformation(EAbilityID AbilityID, FAbilityDetails& AbilityInfo, const FName& InRowName);
	bool GetStatTablesAndIndex(EAbilityID AbilityID, const TArray<FAbilityStatTable>*& OutStatTables, int32& OutArrayIndex) const;

	void UseSpecificAbility(FAbilityDetails& AbilityInfo, TArray<FAbilityStatTable>& AbilityStat, float& Cooldown, float& LastUseTime, float CurrentTime, EAbilityID AbilityID);
	void UpdateAbilityUpgradeStatus(EAbilityID AbilityID, FAbilityDetails& AbilityInfo, int32 InNewCurrentLevel);

	void SetupAbilityCooldown(FAbilityDetails& AbilityInfo, const TArray<FAbilityStatTable>& AbilityStat, int AbilityHaste, float& MaxCooldown, float& CurrentCooldown, FTimerHandle& TimerHandle);
	void HandleAbilityReady(EAbilityID AbilityID);
	void NotifyCooldownChanged(EAbilityID AbilityID, float MaxCooldown, float CurrentCooldown);

	float GetUniqueValue(EAbilityID AbilityID, const FString& InKey);

	UPROPERTY()
	TObjectPtr<class UAOSGameInstance> GameInstance;

	UPROPERTY()
	TObjectPtr<class ACharacterBase> OwnerCharacter;

	UPROPERTY()
	TWeakObjectPtr<class UStatComponent> StatComponent;

public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	FAbilityDetails Ability_Q_Info;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	FAbilityDetails Ability_E_Info;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	FAbilityDetails Ability_R_Info;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	FAbilityDetails Ability_LMB_Info;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	FAbilityDetails Ability_RMB_Info;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	TArray<FAbilityStatTable> Ability_Q_Stat;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	TArray<FAbilityStatTable> Ability_E_Stat;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	TArray<FAbilityStatTable> Ability_R_Stat;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	TArray<FAbilityStatTable> Ability_LMB_Stat;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess = "true"))
	TArray<FAbilityStatTable> Ability_RMB_Stat;

private:
	FTimerHandle Ability_Ban_Timer;

	TMap<EAbilityID, FTimerHandle> CooldownTimers;
	TMap<int32, FTimerHandle> GameTimers;

	FName RowName;
	int32 ChampionIndex = 0;

	ICharacterDataProviderInterface* DataProvider;
};