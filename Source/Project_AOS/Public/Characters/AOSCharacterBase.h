#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase.h"
#include "InputActionValue.h"
#include "Props/ArrowBase.h"
#include "Structs/DamageInfomationStruct.h"
#include "AOSCharacterBase.generated.h"

class UAbilityStatComponent;
class UCharacterRotatorComponent;

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
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Initialization functions
	virtual void InitializeAbilityMontages() {};
	virtual void InitializeAbilityParticles();
	virtual void InitializeAbilityMeshes() {};

	virtual void OnRep_CharacterStateChanged() override;

	void RegisterAbilityStage(EAbilityID AbilityID, int32 Stage, FAbilityStageFunction AbilityFunction);
	void ExecuteAbilityStages(EAbilityID AbilityID);

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

	// Ability check functions
	UFUNCTION()
	virtual void Ability_Q_CheckHit() {};

	UFUNCTION()
	virtual void Ability_E_CheckHit() {};

	UFUNCTION()
	virtual void Ability_R_CheckHit() {};

	UFUNCTION()
	virtual void Ability_LMB_CheckHit() {};

	UFUNCTION()
	virtual void Ability_RMB_CheckHit() {};

protected:
	// Ability execution functions
	UFUNCTION()
	virtual void Ability_Q() {};

	UFUNCTION()
	virtual void Ability_E() {};

	UFUNCTION()
	virtual void Ability_R() {};

	UFUNCTION()
	virtual void Ability_LMB() {};

	UFUNCTION()
	virtual void Ability_RMB() {};

	// Particle and mesh spawning
	UFUNCTION(Server, Reliable)
	void SpawnRootedParticleAtLocation_Server(UParticleSystem* Particle, FTransform Transform);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnRootedParticleAtLocation_Multicast(UParticleSystem* Particle, FTransform Transform);

	UFUNCTION(Server, Reliable)
	void SpawnAttachedParticleAtLocation_Server(UParticleSystem* Particle, FTransform Transform);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnAttachedParticleAtLocation_Multicast(UParticleSystem* Particle, FTransform Transform);

	UFUNCTION(Server, Reliable)
	void SpawnAttachedMeshAtLocation_Server(UStaticMesh* MeshToSpawn, FVector Location, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnAttachedMeshAtLocation_Multicast(UStaticMesh* MeshToSpawn, FVector Location, float Duration);

	UFUNCTION(Server, Reliable)
	void SpawnActorAtLocation_Server(UClass* SpawnActor, FTransform SpawnTransform);

	// Character state updates
	UFUNCTION(Server, Unreliable)
	void UpdateCharacterState_Server(EBaseCharacterState InCharacterState);

	UFUNCTION(Server, Unreliable)
	void UpdateInputValue_Server(const float& InForwardInputValue, const float& InRightInputValue);

	UFUNCTION(Server, Unreliable)
	void UpdateAimValue_Server(const float& InAimPitchValue, const float& InAimYawValue);

public:
	// Damage-related functions
	UFUNCTION()
	virtual void ApplyCriticalHitDamage(FDamageInfomation& DamageInfomation);

	UFUNCTION(Server, Reliable)
	void ApplyDamage_Server(ACharacterBase* Enemy, FDamageInfomation DamageInfomation, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ApplayCrowdControl_Server(ACharacterBase* Enemy, EBaseCrowdControl InCondition = EBaseCrowdControl::None, float InDuration = 0.f, float InPercent = 0.f);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void ApplayCrowdControl_NetMulticast(ACharacterBase* Enemy, EBaseCrowdControl InCondition = EBaseCrowdControl::None, float InDuration = 0.f, float InPercent = 0.f);

	// Montage-related functions
	UFUNCTION(Server, Reliable)
	void StopAllMontages_Server(float BlendOut);

	UFUNCTION(NetMulticast, Reliable)
	void StopAllMontages_NetMulticast(float BlendOut);

	UFUNCTION(Server, Reliable)
	void PlayMontage_Server(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void PlayMontage_NetMulticast(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(Server, Reliable)
	void MontageJumpToSection_Server(UAnimMontage* Montage, FName SectionName, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void MontageJumpToSection_NetMulticast(UAnimMontage* Montage, FName SectionName, float PlayRate);

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
	float SetAnimPlayRate(const float AnimLength);

	UFUNCTION()
	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) {};

	UFUNCTION(Server, Reliable)
	void EnableCharacterMove();

	UFUNCTION(Server, Reliable)
	void EnableSwitchAction();

	UFUNCTION()
	void EnableUseControllerRotationYaw();

	UFUNCTION(Server, Reliable)
	virtual void ServerNotifyAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent);

private:
	// 각 군중 제어 상태를 적용하는 함수들
	void ApplyMovementSpeedDebuff(float Percent, float Duration);
	void ApplyAttackSpeedDebuff(float Percent, float Duration);
	void ApplySilenceDebuff(float Duration);
	void ApplyBlindDebuff(float Duration);
	void ApplySnareDebuff(float Duration);
	void ApplyStunDebuff(float Duration);

public:
	// Additional functions
	void SaveCharacterTransform();
	void SetGameTimer(TMap<int32, FTimerHandle>& Timers, int32 TimerID, TFunction<void()> Callback, float Duration = 1.0f, bool bLoop = false, float FirstDelay = -1.0f);
	void ClearGameTimer(TMap<int32, FTimerHandle>& Timers, int32 TimerID);

	/* returns the Montage Section Attack1~4 */
	const FName GetAttackMontageSection(const int32& Section);

	// Widget functions
	void SetWidgetVisibility(bool Visibility);
	void CheckOutOfSight();

	// Getters
	float GetForwardInputValue() const { return ForwardInputValue; }
	float GetRightInputValue() const { return RightInputValue; }
	float GetAimPitchValue() const { return CurrentAimPitch; };
	float GetAimYawValue() const { return CurrentAimYaw; };
	float GetRootYawOffset() const { return RootYawOffset; };
	void SetWidget(UUserWidgetBase* InUserWidgetBase) override;
	void GetCrowdControl(EBaseCrowdControl InCondition = EBaseCrowdControl::None, float InDuration = 0.f, float InPercent = 0.f) override;
	void BindAbilityStatComponent(UAbilityStatComponent* InAbilityStatComponent) { AbilityStatComponent = InAbilityStatComponent; };
	UAbilityStatComponent* GetAbilityStatComponent() const { return AbilityStatComponent; }

protected:
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

	// Animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_Q_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_E_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_R_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_LMB_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Ability_RMB_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Stun_Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Montage", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAnimMontage> Death_Montage;

	// Effects
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* LevelUpParticle;

	// UI
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|UI", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_StateBar> StateBar;

	// Game references
	UPROPERTY()
	TObjectPtr<class UPlayerAnimInstance> AnimInstance;

	UPROPERTY()
	TObjectPtr<class AAOSGameState> AOSGameState;

	UPROPERTY()
	TObjectPtr<class AAOSPlayerState> AOSPlayerState;

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

	// Timers
	TMap<int32, FTimerHandle> TimerHandleList;
	TMap<int32, FTimerHandle> AbilityTimer;
	TMap<int32, FTimerHandle> CrowdControlTimer;

	FTimerHandle HPReganTimer;
	FTimerHandle MPReganTimer;
	FTimerHandle Ability_Q_Timer;
	FTimerHandle Ability_E_Timer;
	FTimerHandle Ability_R_Timer;
	FTimerHandle Ability_LMB_Timer;
	FTimerHandle Ability_RMB_Timer;
	FTimerHandle DistanceCheckTimerHandle;

	// Character state
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Condition", Meta = (AllowPrivateAccess))
	EBaseCrowdControl CrowdControlState;

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

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim", Meta = (AllowPrivateAccess))
	float RootYawOffset;

	float PreviousRootYawOffset = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Meta = (AllowPrivateAccess))
	bool bIsDead = false;

	bool bCtrlKeyPressed = false;
	bool bIsItemShopOpen = false;
	bool bIsCtrlKeyPressed;

	int32 SelectedCharacterIndex = -1;
	int32 TotalAttacks = 0;
	int32 CriticalHits = 0;    // 전체 크리티컬 히트 수
	uint8 Index : 3;

	float Ability_LMB_PlayRate;
	float Ability_LMB_AnimLength;

	TMap<EAbilityID, TMap<int32, TArray<FAbilityStageFunction>>> AbilityStageMap;
	TArray<FOverlapResult> OutHits;

	FRotator LastCharacterRotation;
	FVector LastCharacterLocation;
	FVector LastForwardVector;
	FVector LastRightVector;
	FVector LastUpVector;
};
