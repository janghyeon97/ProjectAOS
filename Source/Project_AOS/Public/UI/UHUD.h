// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UHUD.generated.h"

// Delegate declaration
DECLARE_MULTICAST_DELEGATE(FOnComponentsBindingCompletedDelegate);


class UStatComponent;
class UAbilityStatComponent;
class AAOSPlayerState;
class AAOSGameState;

/**
 * User HUD class for displaying player information and stats
 */
UCLASS()
class PROJECT_AOS_API UUHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

public:
	// Functions for updating HUD elements
	UFUNCTION()
	void DecreaseHP();

	UFUNCTION()
	void DecreaseMP();

	UFUNCTION()
	void IncreaseEXP();

	UFUNCTION()
	void IncreaseLevel();

	UFUNCTION()
	void IncreaseCriticalChance();

	UFUNCTION()
	void IncreaseAttackSpeed();

	UFUNCTION()
	void ChangeTeamSide(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void UpdateAbilityVisibility(EAbilityID AbilityID, bool InVisibility);

	UFUNCTION()
	void UpdateUpgradeWidgetVisibility(EAbilityID AbilityID, bool InVisibility);

	UFUNCTION()
	void InitializeAbilityLevel(EAbilityID AbilityID);

	UFUNCTION()
	void UpdateAbilityLevel(EAbilityID AbilityID, int InLevel);

	UFUNCTION()
	void UpdateAbilityQTimerText(float MaxCooldown, float CurrentCooldown);

	UFUNCTION()
	void UpdateAbilityETimerText(float MaxCooldown, float CurrentCooldown);

	UFUNCTION()
	void UpdateAbilityRTimerText(float MaxCooldown, float CurrentCooldown);

	UFUNCTION()
	void UpdateAbilityLMBTimerText(float MaxCooldown, float CurrentCooldown);

	UFUNCTION()
	void UpdateAbilityRMBTimerText(float MaxCooldown, float CurrentCooldown);

	UFUNCTION()
	void UpdateMaxHPText(float InOldMaxHP, float InNewMaxHP);

	UFUNCTION()
	void UpdateCurrentHPText(float InOldHP, float InNewHP);

	UFUNCTION()
	void UpdateAttackDamageText(float InOldAD, float InNewAD);

	UFUNCTION()
	void UpdateDefensePowerText(float InOldDP, float InNewDP);

	UFUNCTION()
	void UpdateMagicResistanceText(float InOldMR, float InNewMR);

	UFUNCTION()
	void UpdateAttackSpeedText(float InOldAS, float InNewAS);

	UFUNCTION()
	void UpdateCriticalChanceText(int32 InOldCC, int32 InNewCC);

	UFUNCTION()
	void UpdateMovementSpeedText(float InOldMS, float InNewMS);

	UFUNCTION()
	void UpdateHPRegenText(float InOldHPRegen, float InNewHPRegen);

	UFUNCTION()
	void UpdateMaxMPText(float InOldMaxMP, float InNewMaxMP);

	UFUNCTION()
	void UpdateCurrentMPText(float InOldMP, float InNewMP);

	UFUNCTION()
	void UpdateMPRegenText(float InOldMPRegen, float InNewMPRegen);

	UFUNCTION()
	void UpdateChampionText(const FName& NewString);

	UFUNCTION()
	void UpdateInventory(const TArray<FItemInformation>& Items);

	UFUNCTION()
	void UpdateCurrencyText(const int32 NewCurrency);

	UFUNCTION()
	virtual void UpdateLevelText(int32 InOldLevel, int32 InNewLevel);

	UFUNCTION()
	virtual void UpdateDescriptionText(FName InString);

	UFUNCTION()
	virtual void UpdateRespawnTimeText(int32 InPlayerIndex, float RemainingTime);

public:
	void InitializeHUD(const int32 InChampionIndex, const FName& InChampionName);
	void BindComponents(AAOSGameState* InGameState, AAOSPlayerState* InPlayerState, UStatComponent* InStatComponent, UAbilityStatComponent* InAbilityStatComponent);

	// Getter and Setter for OwningActor
	AActor* GetOwningActor() const { return OwningActor; }
	void SetOwningActor(AActor* InOwningActor) { OwningActor = InOwningActor; }

private:
	void InitializeImages();
	void InitializeAbilityLevels(EAbilityID AbilityID);

	void BindStatComponent(UStatComponent* InStatComponent);
	void BindAbilityStatComponent(UAbilityStatComponent* InAbilityStatComponent);
	void BindPlayerState(AAOSPlayerState* InPlayerState);
	void BindGameState(AAOSGameState* InGameState);

public:
	// Delegate for initialization completion
	FOnComponentsBindingCompletedDelegate OnComponentsBindingCompleted;

protected:
	// Weak pointers to components
	TWeakObjectPtr<UStatComponent> StatComponent;
	TWeakObjectPtr<UAbilityStatComponent> AbilityStatComponent;
	TWeakObjectPtr<AAOSPlayerState> PlayerState;
	TWeakObjectPtr<AAOSGameState> GameState;

	// -----------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD")
	TObjectPtr<AActor> OwningActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> ChampionImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> ChampionImageBorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> ChampionNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> LevelBackgroundImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> LevelText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> RespawnTimeImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> RespawnTimeText;

	// Buttons
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UButton> DecreaseHPButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UButton> DecreaseMPButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UButton> IncreaseEXPButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UButton> IncreaseLevelButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UButton> IncreaseCriticalButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UButton> IncreaseAttackSpeedButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UComboBoxString> TeamSelectionComboBox;

	// Images for level-up abilities
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> LevelUp_Q_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> LevelUp_E_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> LevelUp_R_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> LevelUp_LMB_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> LevelUp_RMB_Image;

	// Text blocks for various stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> DescriptionText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> CurrentHPText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> MaxHPText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> CurrentMPText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> MaxMPText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> HPRegenText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> MPRegenText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> AttackDamageText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> AbilityPowerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> DefensePowerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> MagicResistanceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> AttackSpeedText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> CooldownReductionText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> CriticalChanceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> MovementSpeedText;

	// Ability images and cooldown texts
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> Ability_Q_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> Ability_Q_CooldownText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UBorder> Ability_Q_Border;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> Ability_E_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> Ability_E_CooldownText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UBorder> Ability_E_Border;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> Ability_R_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> Ability_R_CooldownText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UBorder> Ability_R_Border;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> Ability_LMB_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> Ability_LMB_CooldownText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UBorder> Ability_LMB_Border;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UImage> Ability_RMB_Image;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> Ability_RMB_CooldownText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UBorder> Ability_RMB_Border;

	// Ability levels
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_AbilityLevel> Ability_Q_Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_AbilityLevel> Ability_E_Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_AbilityLevel> Ability_R_Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_AbilityLevel> Ability_LMB_Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_AbilityLevel> Ability_RMB_Level;

	// Progress bars
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_HPBar> HPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_MPBar> MPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_EXPBar> EXPBar;

	// Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<class UUW_Inventory> Inventory;


private:
	TArray<UMaterialInstanceDynamic*> MaterialRef;

	UMaterialInstanceDynamic* RespawnTimeImageMaterialRef;
	UMaterialInstanceDynamic* CharacterImageMaterialRef;
	UMaterialInstanceDynamic* LevelBackgroundMaterialRef;

	FName PlayerCharacterName;
	int32 PlayerIndex = 0;

	FName ChampionName;
	int32 ChampionIndex = 0;
};
