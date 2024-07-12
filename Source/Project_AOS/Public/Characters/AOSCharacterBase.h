// Fill out your copyright notice in the Description page of Project Settings.

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
struct FCircularParticleInfomation
{
	GENERATED_BODY()

public:
	FCircularParticleInfomation()
		: ParticleID(-1)
		, Iterator(0)
		, NumOfParicles(0)
		, Angle(0.0f)
		, Radius(0.0f)
		, Lifetime(0.0f)
		, Rate(0.0f)
		, Delay(0.0f)
		, ForwardVector(FVector::ZeroVector)
		, RightVector(FVector::ZeroVector)
		, UpVector(FVector::ZeroVector)
		, Transform(FTransform::Identity)
		, Particle(nullptr)
	{
	}

	bool operator==(const FCircularParticleInfomation& A) const
	{
		return this->ParticleID == A.ParticleID;
	}

	// 유효성 검사 메서드
	bool IsValid() const
	{
		return ParticleID >= 0 && Particle != nullptr;
	}

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 ParticleID;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int Iterator;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int NumOfParicles;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float Angle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float Radius;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float Lifetime;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float Rate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float Delay;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FVector ForwardVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FVector RightVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FVector UpVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FTransform Transform;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	UParticleSystem* Particle;
};


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

public:
	// Initialization functions
	virtual void InitializeAbilityMontages() {};
	virtual void InitializeAbilityParticles();
	virtual void InitializeAbilityMeshes() {};

public:
	UFUNCTION()
	virtual void Respawn();

	UFUNCTION()
	void SpawnLevelUpParticle(int32 OldLevel, int32 NewLevel);

	// 능력 업그레이드
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

	// Ctrl 키 입력 처리
	UFUNCTION()
	void HandleCtrlKeyInput(bool bPressed);

	UFUNCTION()
	void OnCtrlKeyPressed();

	UFUNCTION()
	void OnCtrlKeyReleased();

	// 아이템 사용 
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

	// HUD 함수
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

	// 능력 체크 함수
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
	// 능력 실행 함수
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

	// 파티클 및 메쉬 스폰
	UFUNCTION(Server, Reliable)
	void SpawnRootedParicleAtLocation_Server(UParticleSystem* Particle, FTransform transform);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnRootedParicleAtLocation_Multicast(UParticleSystem* Particle, FTransform transform);

	UFUNCTION(Server, Reliable)
	void SpawnAttachedParicleAtLocation_Server(UParticleSystem* Particle, FTransform transform);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnAttachedParicleAtLocation_Multicast(UParticleSystem* Particle, FTransform transform);

	UFUNCTION(Server, Reliable)
	void SpawnCircularParicleAtLocation_Server(UParticleSystem* Particle, FCircularParticleInfomation CircularParticleInfomation, FTransform transform);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnCircularParicleAtLocation_Multicast(UParticleSystem* Particle, FCircularParticleInfomation CircularParticleInfomation, FTransform transform);

	UFUNCTION(Server, Reliable)
	void SpawnAttachedMeshAtLocation_Server(UStaticMesh* MeshToSpawn, FVector Location, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnAttachedMeshAtLocation_Multicast(UStaticMesh* MeshToSpawn, FVector Location, float Duration);

	UFUNCTION(Server, Reliable)
	void SpawnActorAtLocation_Server(UClass* SpawnActor, FTransform SpawnTransform);

	// 캐릭터 상태 업데이트
	UFUNCTION(Server, Unreliable)
	void UpdateCharacterState_Server(EBaseCharacterState InCharacterState);

	UFUNCTION(Server, Unreliable)
	void UpdateInputValue_Server(const float& InForwardInputValue, const float& InRightInputValue);

	UFUNCTION(Server, Unreliable)
	void UpdateAimValue_Server(const float& InAimPitchValue, const float& InAimYawValue);

public:
	// 데미지 관련 함수들
	UFUNCTION()
	virtual void ApplyCriticalHitDamage(FDamageInfomation& DamageInfomation);

	UFUNCTION(Server, Reliable)
	void ApplyDamage_Server(ACharacterBase* Enemy, FDamageInfomation DamageInfomation, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ApplayCrowdControl_Server(ACharacterBase* Enemy, ECrowdControlBase InCondition = ECrowdControlBase::None, float InDuration = 0.f, float InPercent = 0.f);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void ApplayCrowdControl_NetMulticast(ACharacterBase* Enemy, ECrowdControlBase InCondition = ECrowdControlBase::None, float InDuration = 0.f, float InPercent = 0.f);

	// 몽타주 관련 함수
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

	// HP 및 MP 재생 관련 함수
	UFUNCTION(Server, Reliable)
	void HPRegenEverySecond_Server();

	UFUNCTION(Server, Reliable)
	void MPRegenEverySecond_Server();

	UFUNCTION(Server, Reliable)
	void ClearHPRegenTimer_Server();

	UFUNCTION(Server, Reliable)
	void ClearMPRegenTimer_Server();


	// 캐릭터 사망 시
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

	UFUNCTION()
	void EnableCharacterMove();

	UFUNCTION()
	void EnableSwitchAtion();

	UFUNCTION()
	void EnableUseControllerRotationYaw();

private:
	// 각 군중 제어 상태를 적용하는 함수들
	void ApplyMovementSpeedDebuff(float Percent, float Duration);
	void ApplyAttackSpeedDebuff(float Percent, float Duration);
	void ApplySilenceDebuff(float Duration);
	void ApplyBlindDebuff(float Duration);
	void ApplyStunDebuff(float Duration);

public:
	/* returns the Montage Section Attack1~4 */
	const FName GetAttackMontageSection(const int32& Section);

	// 위젯 관련 함수
	void SetWidgetVisibility(bool Visibility);
	void CheckOutOfSight();

	float GetForwardInputValue() const { return ForwardInputValue; }
	float GetRightInputValue() const { return RightInputValue; }
	float GetAimPitchValue() const { return CurrentAimPitch; };
	float GetAimYawValue() const { return CurrentAimYaw; };
	float GetRootYawOffset() const { return RootYawOffset; };
	void SetWidget(UUserWidgetBase* InUserWidgetBase) override;
	void GetCrowdControl(ECrowdControlBase InCondition = ECrowdControlBase::None, float InDuration = 0.f, float InPercent = 0.f) override;
	void BindAbilityStatComponent(UAbilityStatComponent* InAbilityStatComponent) { AbilityStatComponent = InAbilityStatComponent; };
	UAbilityStatComponent* GetAbilityStatComponent() const { return AbilityStatComponent; }

protected:
	// 애니메이션 참조
	UPROPERTY()
	TObjectPtr<class UPlayerAnimInstance> AnimInstance;

	// 게임 상태 참조
	UPROPERTY()
	TObjectPtr<class AAOSGameState> AOSGameState;

	UPROPERTY()
	TObjectPtr<class AAOSPlayerState> AOSPlayerState;

	// 입력 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputConfigData> PlayerCharacterInputConfigData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputMappingContext> PlayerInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|InputConfig", Meta = (AllowPrivateAccess))
	TObjectPtr<class UInputMappingContext> PlayerCtrlInputMappingContext;

	// 애니메이션 몽타주
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

	// 파티클 시스템
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

	// Particles
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* LevelUpParticle;

	// UI 관련
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|UI", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_StateBar> StateBar;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|HitResults", Meta = (AllowPrivateAccess))
	TObjectPtr<class ACharacterBase> HitResults;


	UPROPERTY(Replicated, Transient, VisibleAnywhere, Category = "Character|Condition", Meta = (AllowPrivateAccess))
	ECrowdControlBase CrowdControl;

	UPROPERTY()
	TArray<FOverlapResult> OutHits;

	UPROPERTY()
	FTimerHandle HPReganTimer;

	UPROPERTY()
	FTimerHandle MPReganTimer;

	UPROPERTY()
	FTimerHandle CrowdControlTimer;

	// 캐릭터 이동
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float ForwardInputValue;

	float PreviousForwardInputValue = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement", Meta = (AllowPrivateAccess))
	float RightInputValue;

	float PreviousRightInputValue = 0.f;

	// 캐릭터 조준
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim", Meta = (AllowPrivateAccess))
	float CurrentAimPitch = 0.f;

	float PreviousAimPitch = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim", Meta = (AllowPrivateAccess))
	float CurrentAimYaw = 0.f;

	float PreviousAimYaw = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|Aim", Meta = (AllowPrivateAccess))
	float RootYawOffset;

	float PreviousRootYawOffset = 0.f;

	// 캐릭터 선택
	UPROPERTY()
	int32 SelectedCharacterIndex = -1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Character|State", Meta = (AllowPrivateAccess))
	bool bIsDead = false;

protected:
	// 입력 컴포넌트
	UPROPERTY()
	class UEnhancedInputComponent* EnhancedInputComponent;

	UPROPERTY()
	class APostProcessVolume* PostProcessVolume;

	// 타이머 핸들 리스트
	TArray<FTimerHandle> TimerHandleList;
	TArray<FCircularParticleInfomation> ParticleInfoList;

	uint8 Index : 3;

	int32 TotalAttacks = 0;    // 전체 기본 공격 횟수
	int32 CriticalHits = 0;    // 전체 크리티컬 히트 수

	float Ability_LMB_PlayRate;
	float Ability_LMB_AnimLength;

	FRotator LastCharacterRotation;
	FVector LastCharacterLocation;
	FVector LastForwardVector;
	FVector LastRightVector;
	FVector LastUpVector;

	bool bIsCtrlKeyPressed;

	// 능력 타이머
	FTimerHandle Ability_Q_Timer;
	FTimerHandle Ability_E_Timer;
	FTimerHandle Ability_R_Timer;
	FTimerHandle Ability_LMB_Timer;
	FTimerHandle Ability_RMB_Timer;
	FTimerHandle DistanceCheckTimerHandle;

	bool bCtrlKeyPressed = false;
	bool bIsItemShopOpen = false;
};
