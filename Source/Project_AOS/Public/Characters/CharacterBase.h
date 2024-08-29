// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Structs/EnumTeamSide.h"
#include "Structs/EnumCrowdControl.h"
#include "Structs/EnumCharacterType.h"
#include "Structs/EnumCharacterState.h"
#include "Structs/CustomCombatData.h"
#include "CharacterBase.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceiveDamageEnteredDelegate, bool&, bResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreReceiveDamageDelegate, FDamageInformation&, DamageInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostReceiveDamageDelegate, float&, FinalDamage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreDamageCalculationDelegate, FDamageInformation&, DamageInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostDamageCalculationDelegate, float&, FinalDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreDeathDelegate, bool&, bDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreCrowdControlDelegate, bool&, bResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitEventTriggeredDelegate, FDamageInformation&, DamageInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackEventTriggeredDelegate, FDamageInformation&, DamageInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEffectsEventTriggeredDelegate, FDamageInformation&, DamageInformation);

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
	// Initialization and Resource Functions
	virtual void InitializeCharacterResources();
	virtual void SetWidget(class UUserWidgetBase* InUserWidgetBase);

	UAnimMontage* GetOrLoadMontage(const FName& Key, const TCHAR* Path);
	UParticleSystem* GetOrLoadParticle(const FName& Key, const TCHAR* Path);
	UStaticMesh* GetOrLoadMesh(const FName& Key, const TCHAR* Path);
	UParticleSystem* GetOrLoadSharedParticle(const FName& Key, const TCHAR* Path);

	// Ability Execution Functions
	UFUNCTION()
	virtual void Ability_Q();
	UFUNCTION()
	virtual void Ability_E();
	UFUNCTION()
	virtual void Ability_R();
	UFUNCTION()
	virtual void Ability_LMB();
	UFUNCTION()
	virtual void Ability_RMB();

	// Ability Check Functions
	UFUNCTION()
	virtual void Ability_Q_CheckHit();
	UFUNCTION()
	virtual void Ability_E_CheckHit();
	UFUNCTION()
	virtual void Ability_R_CheckHit();
	UFUNCTION()
	virtual void Ability_LMB_CheckHit();
	UFUNCTION()
	virtual void Ability_RMB_CheckHit();

	// Ability Cancel Functions
	UFUNCTION()
	virtual void CancelAbility();
	UFUNCTION()
	virtual void Ability_Q_Canceled();
	UFUNCTION()
	virtual void Ability_E_Canceled();
	UFUNCTION()
	virtual void Ability_R_Canceled();
	UFUNCTION()
	virtual void Ability_LMB_Canceled();
	UFUNCTION()
	virtual void Ability_RMB_Canceled();

	bool ValidateHit(EAbilityID AbilityID);

	// Damage-related Functions
	UFUNCTION()
	virtual bool ReceiveDamage(FDamageInformation DamageInformation, AController* EventInstigator, AActor* DamageCauser);
	UFUNCTION()
	virtual void ApplyCriticalHitDamage(FDamageInformation& DamageInformation);
	UFUNCTION(Server, Reliable)
	void ApplyDamage_Server(ACharacterBase* Enemy, FDamageInformation DamageInformation, AController* EventInstigator, AActor* DamageCauser);

	// Crowd Control Functions
	virtual void ApplyCrowdControlEffect(FCrowdControlInformation CrowdControlInfo);

	UFUNCTION(Server, Reliable)
	void ApplyCrowdControl_Server(ACharacterBase* Enemy, FCrowdControlInformation CrowdControlInfo);
	UFUNCTION(NetMulticast, Reliable)
	void ApplyCrowdControl_NetMulticast(ACharacterBase* Enemy, FCrowdControlInformation CrowdControlInfo);

	// State Functions
	void LogCharacterState(EBaseCharacterState State, const FString& Context);

	UFUNCTION(Server, Reliable)
	virtual void ModifyCharacterState(ECharacterStateOperation Operation, EBaseCharacterState StateFlag);

	// Miscellaneous Functions
	float GetUniqueAttribute(EAbilityID AbilityID, const FString& Key, float DefaultValue) const;
	float SetAnimPlayRate(const float AnimLength);

	// Movement Functions
	UFUNCTION()
	virtual void ChangeMovementSpeed(float InOldMS, float InNewMS);
	UFUNCTION()
	virtual void OnRep_CharacterStateChanged();
	UFUNCTION()
	virtual void OnRep_CrowdControlStateChanged();

	UFUNCTION()
	void OnRep_UseControllerRotationYaw();

	// Delegate Functions
	UFUNCTION()
	virtual void OnPreCalculateDamage(float& AttackDamage, float& AbilityPower);
	UFUNCTION()
	virtual void OnPreDamageReceived(float FinalDamage);

protected:
	// Particle and mesh spawning
	UFUNCTION(Server, Reliable)
	void SpawnRootedParticleAtLocation_Server(UParticleSystem* Particle, FTransform Transform);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnRootedParticleAtLocation_Multicast(UParticleSystem* Particle, FTransform Transform);

	UFUNCTION(Server, Reliable)
	void SpawnAttachedParticleAtLocation_Server(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnAttachedParticleAtLocation_Multicast(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType);

	UFUNCTION(Server, Reliable)
	void SpawnAttachedMeshAtLocation_Server(UStaticMesh* MeshToSpawn, FVector Location, float Duration);

	UFUNCTION(NetMulticast, Reliable)
	void SpawnAttachedMeshAtLocation_Multicast(UStaticMesh* MeshToSpawn, FVector Location, float Duration);

	UFUNCTION(Server, Reliable)
	void SpawnActorAtLocation_Server(UClass* SpawnActor, FTransform SpawnTransform);

	UFUNCTION(Client, Reliable)
	void SpawnDamageWidget_Client(AActor* Target, const float DamageAmount, const FDamageInformation& DamageInformation);

	UFUNCTION()
	void OnDamageWidgetAnimationFinished();

public:
	// Montage-related functions
	void PlayMontage(const FString& MontageName, float PlayRate = 1.0f, FName StartSectionName = NAME_None, const TCHAR* Path = nullptr);

	UFUNCTION(Server, Reliable)
	void PlayMontage_Server(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSectionName = NAME_None, bool bApplyToLocalPlayer = false);

	UFUNCTION(NetMulticast, Reliable)
	void PlayMontage_NetMulticast(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSectionName = NAME_None, bool bApplyToLocalPlayer = false);

	UFUNCTION(Server, Reliable)
	void StopAllMontages_Server(float BlendOut, bool bApplyToLocalPlayer = false);

	UFUNCTION(NetMulticast, Reliable)
	void StopAllMontages_NetMulticast(float BlendOut, bool bApplyToLocalPlayer = false);

	UFUNCTION(Server, Reliable)
	void MontageJumpToSection_Server(const UAnimMontage* Montage, FName SectionName);

	UFUNCTION(NetMulticast, Reliable)
	void MontageJumpToSection_NetMulticast(const UAnimMontage* Montage, FName SectionName);

	UFUNCTION(NetMulticast, Reliable)
	void PauseMontage_NetMulticast();

	UFUNCTION(NetMulticast, Reliable)
	void ResumeMontage_NetMulticast();

public:
	// Ability Stat Component Getter
	class UStatComponent* GetStatComponent() const;
	class UAbilityStatComponent* GetAbilityStatComponent() const;

	// Delegates
	FOnReceiveDamageEnteredDelegate OnReceiveDamageEnteredEvent;
	FOnPreDamageCalculationDelegate OnPreDamageCalculationEvent;
	FOnPostDamageCalculationDelegate OnPostDamageCalculationEvent;
	FOnPreReceiveDamageDelegate OnPreReceiveDamageEvent;
	FOnPostReceiveDamageDelegate OnPostReceiveDamageEvent;
	FOnHitEventTriggeredDelegate OnHitEventTriggered;
	FOnAttackEventTriggeredDelegate OnAttackEventTriggered;
	FOnAbilityEffectsEventTriggeredDelegate OnAbilityEffectsEventTriggered;
	FOnPreDeathDelegate OnPreDeathEvent;
	FOnPreCrowdControlDelegate OnPreCrowdControlEvent;

	// Character Resources
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Resource", Meta = (AllowPrivateAccess))
	TMap<FName, UAnimMontage*> CharacterAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Resource", Meta = (AllowPrivateAccess))
	TMap<FName, UParticleSystem*> CharacterParticles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Resource", Meta = (AllowPrivateAccess))
	TMap<FName, UStaticMesh*> CharacterMeshes;

	static TMap<FName, UParticleSystem*> SharedGameplayParticles;

	// 캐릭터 상태
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Character|TeamSide")
	ETeamSideBase TeamSide = ETeamSideBase::Type;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	EObjectType ObjectType = EObjectType::None;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterStateChanged, Transient, VisibleAnywhere, Category = "CharacterState")
	EBaseCharacterState CharacterState;

	UPROPERTY(ReplicatedUsing = OnRep_CrowdControlStateChanged, Transient, VisibleAnywhere, Category = "CharacterState")
	EBaseCrowdControl CrowdControlState;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CharacterState")
	bool bIsDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_UseControllerRotationYaw)
	bool bReplicatedUseControllerRotationYaw;

	// AI 캐릭터 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MinionBase")
	float MaxChaseDistanceFromSpline = 1000.f;

protected:
	// Components
	UPROPERTY()
	TObjectPtr<class UAnimInstance> AnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (AllowPrivateAccess))
	TObjectPtr<class UStatComponent> StatComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AbilityStat", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAbilityStatComponent> AbilityStatComponent;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "AbilityStat", Meta = (AllowPrivateAccess))
	TObjectPtr<class AActor> LastHitActor;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UDamageNumberWidget> DamageNumberWidgetClass;

	// 위젯 컴포넌트들을 관리할 큐
	TQueue<class UWidgetComponent*> DamageWidgetQueue;

	// Variables
	float LastMovementSpeed;
	float LastAttackSpeed;
	int32 TotalAttacks = 0;
	int32 CriticalHits = 0;    // 전체 크리티컬 히트 수

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CrowdControl", Meta = (AllowPrivateAccess))
	TMap<EBaseCrowdControl, class UCrowdControlEffectBase*> ActiveEffects;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CrowdControl", meta = (AllowPrivateAccess = "true"))
	class UCrowdControlManager* CrowdControlManager;
};