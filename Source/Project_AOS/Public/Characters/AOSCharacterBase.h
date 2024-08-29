#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "InputActionValue.h"
#include "Props/ArrowBase.h"
#include "Structs/CustomCombatData.h"
#include "AOSCharacterBase.generated.h"

class UAbilityStatComponent;
class UCharacterRotatorComponent;
class UCharacterDataProviderBase;
class UChampionDataProvider;

USTRUCT(BlueprintType)
struct FParticleTransformInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale;

	FParticleTransformInfo() : Location(FVector::ZeroVector), Rotation(FRotator::ZeroRotator), Scale(FVector::ZeroVector) {}
	FParticleTransformInfo(FVector InLocation, FRotator InRotation, FVector InScale) : Location(InLocation), Rotation(InRotation), Scale(InScale) {}
};


USTRUCT(BlueprintType)
struct FImpactResult
{
	GENERATED_BODY()

public:
	FImpactResult()
		: bHit(false), ImpactPoint(FVector::ZeroVector) {}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact")
	bool bHit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact")
	FVector ImpactPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact")
	FHitResult HitResult;
};


// Delegate for Ability Functions
DECLARE_DELEGATE(FAbilityStageFunction);

UCLASS()
class PROJECT_AOS_API AAOSCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

	friend class UPlayerAnimInstance;

public:
	AAOSCharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_CharacterStateChanged() override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void InitializeCharacterResources() override;

	// Utility Functions
	void SaveCharacterTransform();
	void SetGameTimer(TMap<int32, FTimerHandle>& Timers, int32 TimerID, TFunction<void()> Callback, float Duration = 1.0f, bool bLoop = false, float FirstDelay = -1.0f);
	void ClearGameTimer(TMap<int32, FTimerHandle>& Timers, int32 TimerID);
	bool IsGameTimerActive(TMap<int32, FTimerHandle>& Timers, int32 ItemID) const;
	const FName GetAttackMontageSection(const int32& Section);
	void SetWidgetVisibility(bool Visibility);
	void CheckOutOfSight();
	void UpdateOverlayMaterial();
	FImpactResult GetImpactPoint(const float TraceRange = 10000.f);
	FImpactResult GetSweepImpactPoint(const float TraceRange = 10000.f);

public:
	// Getters and Setters
	float GetForwardInputValue() const { return ForwardInputValue; }
	float GetRightInputValue() const { return RightInputValue; }
	float GetAimPitchValue() const { return CurrentAimPitch; }
	float GetAimYawValue() const { return CurrentAimYaw; }
	virtual void SetWidget(UUserWidgetBase* InUserWidgetBase) override;
	void BindAbilityStatComponent(UAbilityStatComponent* InAbilityStatComponent) { AbilityStatComponent = InAbilityStatComponent; }

	void DisableJump();
	void EnableJump();

public:
	// Character actions
	UFUNCTION()
	virtual void Respawn();

	UFUNCTION()
	void SpawnLevelUpParticle(int32 OldLevel, int32 NewLevel);

	void UpgradeAbility(EAbilityID AbilityID, TFunction<void(int32)> InitializeAbilityFunction);

	UFUNCTION(Server, Reliable)
	void UpgradeAbility_Q_Server();

	UFUNCTION(Server, Reliable)
	void UpgradeAbility_E_Server();

	UFUNCTION(Server, Reliable)
	void UpgradeAbility_R_Server();

	UFUNCTION(Server, Reliable)
	void UpgradeAbility_LMB_Server();

	UFUNCTION(Server, Reliable)
	void UpgradeAbility_RMB_Server();

	// Item Shop
	void ToggleItemShop();

	// Ctrl key input handling
	UFUNCTION()
	void HandleCtrlKeyInput(bool bPressed);

	UFUNCTION()
	void OnCtrlKeyPressed();

	UFUNCTION()
	void OnCtrlKeyReleased();

	// Recall
	UFUNCTION()
	void Recall();

	// Item usage
	UFUNCTION(Server, Reliable)
	void UseItemSlot1();

	UFUNCTION(Server, Reliable)
	void UseItemSlot2();

	UFUNCTION(Server, Reliable)
	void UseItemSlot3();

	UFUNCTION(Server, Reliable)
	void UseItemSlot4();

	UFUNCTION(Server, Reliable)
	void UseItemSlot5();

	UFUNCTION(Server, Reliable)
	void UseItemSlot6();

	// HUD functions
	UFUNCTION(Server, Unreliable)
	void DecreaseHP_Server();

	UFUNCTION(Server, Unreliable)
	void DecreaseMP_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseEXP_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseLevel_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseCriticalChance_Server();

	UFUNCTION(Server, Unreliable)
	void IncreaseAttackSpeed_Server();

	UFUNCTION(Server, Unreliable)
	void ChangeTeamSide_Server(ETeamSideBase InTeamSide);

protected:
	// Character state updates
	UFUNCTION(Server, Unreliable)
	void UpdateCharacterState_Server(EBaseCharacterState InCharacterState);

	UFUNCTION(Server, Unreliable)
	void UpdateInputValue_Server(const float& InForwardInputValue, const float& InRightInputValue);

	UFUNCTION(Server, Unreliable)
	void UpdateAimValue_Server(const float& InAimPitchValue, const float& InAimYawValue);

protected:
	// HP and MP regeneration
	UFUNCTION(Server, Reliable)
	void HPRegenEverySecond_Server();

	UFUNCTION(Server, Reliable)
	void MPRegenEverySecond_Server();

	UFUNCTION(Server, Reliable)
	void ClearHPRegenTimer_Server();

	UFUNCTION(Server, Reliable)
	void ClearMPRegenTimer_Server();

	// Character death
	UFUNCTION()
	virtual void OnCharacterDeath();

	UFUNCTION(Client, Reliable)
	void ActivatePostProcessEffect_Client();

	UFUNCTION(Client, Reliable)
	void DeActivatePostProcessEffect_Client();

	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) {};

	UFUNCTION()
	void EnableCharacterMove();

	UFUNCTION()
	void EnableSwitchAction();

	UFUNCTION()
	void EnableUseControllerRotationYaw();

	UFUNCTION()
	void EnableGravity();

	UFUNCTION()
	void DisableGravity();

	UFUNCTION(Server, Reliable)
	void ServerNotifyAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent);

	virtual	void OnAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent);

protected:
	// Game references
	UPROPERTY()
	TObjectPtr<class AAOSPlayerState> AOSPlayerState;
	UPROPERTY()
	TObjectPtr<class AAOSGameState> AOSGameState;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class UParticleSystemComponent> ScreenParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TArray<class UStaticMeshComponent*> StaticMeshComponents;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class UCharacterWidgetComponent> WidgetComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Components", Meta = (AllowPrivateAccess))
	TObjectPtr<class UCameraComponent> CameraComponent;

	// UI
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|UI", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_StateBar> StateBar;

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputConfigData> PlayerCharacterInputConfigData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputMappingContext> PlayerInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputMappingContext> PlayerCtrlInputMappingContext;

	UPROPERTY()
	class UEnhancedInputComponent* EnhancedInputComponent;

	UPROPERTY()
	class APostProcessVolume* PostProcessVolume;

	// Character state
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float ForwardInputValue;

	float PreviousForwardInputValue = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float RightInputValue;

	float PreviousRightInputValue = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim", Meta = (AllowPrivateAccess))
	float CurrentAimPitch = 0.f;

	float PreviousAimPitch = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim", Meta = (AllowPrivateAccess))
	float CurrentAimYaw = 0.f;

	float PreviousAimYaw = 0.f;

protected:
	// Timers
	TMap<int32, FTimerHandle> GameTimer;
	TMap<int32, FTimerHandle> AbilityTimer;
	TMap<int32, FTimerHandle> CrowdControlTimer;

	int32 GameTimerIndex = 0;

	FTimerHandle HPReganTimer;
	FTimerHandle MPReganTimer;

	bool bCtrlKeyPressed = false;
	bool bIsItemShopOpen = false;
	bool bIsCtrlKeyPressed;

	FName ChampionName;
	int32 SelectedCharacterIndex = -1;
	uint8 Index : 3;

	float Ability_LMB_PlayRate;
	float Ability_LMB_AnimLength;

	TArray<FOverlapResult> OutHits;

	FRotator LastCharacterRotation;
	FVector LastCharacterLocation;
	FVector LastForwardVector;
	FVector LastRightVector;
	FVector LastUpVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UMaterialInterface* OverlayMaterial_Ally;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UMaterialInterface* OverlayMaterial_Enemy;

	AActor* CurrentTarget;
	UMaterialInterface* OriginalMaterial; // 원래 머테리얼을 저장합니다.
};
