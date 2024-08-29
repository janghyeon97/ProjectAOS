// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/AOSCharacterBase.h"


// Game ���� ��� ����
#include "Game/AOSGameMode.h"
#include "Game/AOSGameState.h"
#include "Game/AOSPlayerState.h"
#include "Game/AOSGameInstance.h"

// Input ���� ��� ����
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/InputConfigData.h"

// Camera ���� ��� ����
#include "Camera/CameraComponent.h"

// Animation ���� ��� ����
#include "Animations/PlayerAnimInstance.h"

// Component ���� ��� ����
#include "Components/CapsuleComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Components/StatComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Controller ���� ��� ����
#include "Controllers/AOSPlayerController.h"

// Kismet ���� ��� ����
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

// UI ���� ��� ����
#include "UI/UW_StateBar.h"

// Timer ���� ��� ����
#include "TimerManager.h"

// Network ���� ��� ����
#include "Net/UnrealNetwork.h"

// Engine ���� ��� ����
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"

#include "DataProviders/CharacterDataProviderBase.h"
#include "DataProviders/ChampionDataProvider.h"


AAOSCharacterBase::AAOSCharacterBase()
{
	WidgetComponent = CreateDefaultSubobject<UCharacterWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetRootComponent());
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));

	static ConstructorHelpers::FClassFinder<UUserWidgetBase> STATE_BAR(TEXT("/Game/ProjectAOS/UI/WBP_StateBar.WBP_StateBar_C"));
	if (STATE_BAR.Succeeded())
	{
		WidgetComponent->SetWidgetClass(STATE_BAR.Class);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(200.0f, 35.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 500.f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bDoCollisionTest = true;
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(0.f, 80.f, 100.f));
	CameraComponent->PostProcessSettings.bOverride_MotionBlurAmount = true;
	CameraComponent->PostProcessSettings.MotionBlurAmount = 0.0f; // ��� �� ��Ȱ��ȭ

	ScreenParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ScreenParticleSystem"));
	ScreenParticleSystem->SetupAttachment(CameraComponent);
	ScreenParticleSystem->SetRelativeLocation(FVector(40.f, 0.f, 0.f));
	ScreenParticleSystem->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->MaxWalkSpeed = 100.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->GravityScale = 1.0;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	float CharacterHalfHeight = 95.f;
	float CharacterRadius = 40.f;
	GetCapsuleComponent()->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);

	FVector PivotPosition(0.f, 0.f, -CharacterHalfHeight);
	FRotator PivotRotation(0.f, -90.f, 0.f);
	GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

	Index = -1;
	ObjectType = EObjectType::Player;

	TotalAttacks = 0;
	CriticalHits = 0;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("AOSCharacter"));

	GetCharacterMovement()->SetIsReplicated(true);

	bIsCtrlKeyPressed = false;
	PrimaryActorTick.bCanEverTick = true;
}

void AAOSCharacterBase::InitializeCharacterResources()
{
	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Invalid GameInstance."));
		return;
	}

	ICharacterDataProviderInterface* Provider = Cast<ICharacterDataProviderInterface>(GameInstance->GetDataProvider(EObjectType::Player));
	if (!Provider)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Invalid DataProvider."));
		return;
	}

	CharacterAnimations = Provider->GetCharacterMontagesMap(ChampionName);
	CharacterParticles = Provider->GetCharacterParticlesMap(ChampionName);
	CharacterMeshes = Provider->GetCharacterMeshesMap(ChampionName);

	// SharedGamePlayParticlesMap�� ��� �ִ� ��쿡�� �ʱ�ȭ
	if (SharedGameplayParticles.Num() == 0)
	{
		const UDataTable* SharedGameplayTable = GameInstance->GetSharedGamePlayParticlesDataTable();
		if (!SharedGameplayTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("SharedGameplayTable is null. Initialization aborted."));
			return;
		}

		// ���̺��� ��� ���� �ݺ��մϴ�.
		for (const auto& Row : SharedGameplayTable->GetRowMap())
		{
			const FSharedGameplay* RowData = reinterpret_cast<const FSharedGameplay*>(Row.Value);
			if (RowData)
			{
				// �� �������� ��� �Ӽ��� �ݺ��Ͽ� �ʿ� �߰��մϴ�.
				for (const auto& Attribute : RowData->SharedGameplayParticles)
				{
					if (!SharedGameplayParticles.Contains(Attribute.Key)) // �ߺ��� Ű�� ���ϱ� ���� Ȯ��
					{
						SharedGameplayParticles.Add(Attribute.Key, Attribute.Value);
					}
				}
			}
		}
		UE_LOG(LogTemp, Log, TEXT("SharedGameplayParticles initialized with %d entries."), SharedGameplayParticles.Num());
	}
}

void AAOSCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		if (PreviousForwardInputValue != ForwardInputValue || PreviousRightInputValue != RightInputValue)
		{
			UpdateInputValue_Server(ForwardInputValue, RightInputValue);
		}
	}
}

//==================== Replication Functions ====================//

void AAOSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		
	DOREPLIFETIME_CONDITION(ThisClass, ForwardInputValue, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, RightInputValue, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentAimPitch, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentAimYaw, COND_SkipOwner);
}

void AAOSCharacterBase::BeginPlay()	
{
	Super::BeginPlay();

	// �������� ����
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
			{
				if (APlayerState* PlayerState = GetPlayerState())
				{
					AOSPlayerState = Cast<AAOSPlayerState>(PlayerState);
					if (::IsValid(AOSPlayerState))
					{
						TeamSide = (AOSPlayerState->TeamSide == ETeamSideBase::Blue) ? ETeamSideBase::Blue : ETeamSideBase::Red;

						StatComponent->OnCurrentLevelChanged.AddDynamic(AOSPlayerState, &AAOSPlayerState::OnPlayerLevelChanged);

						UE_LOG(LogTemp, Warning, TEXT("[AAOSCharacterBase::BeginPlay] AOSPlayerState - StatComponent OnPlayerLevelChanged binding complete."));
					}
				}

				AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(GetController());
				if (::IsValid(PlayerController))
				{
					PlayerController->InitializeHUD(SelectedCharacterIndex, ChampionName);
				}
			}
		));

		if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == false)
		{
			StatComponent->OnOutOfCurrentHP.AddDynamic(this, &ThisClass::OnCharacterDeath);
		}

		if (StatComponent->OnCurrentLevelChanged.IsAlreadyBound(this, &ThisClass::SpawnLevelUpParticle) == false)
		{
			StatComponent->OnCurrentLevelChanged.AddDynamic(this, &ThisClass::SpawnLevelUpParticle);
		}

		// ũ��Ƽ�� ��Ʈ �̺�Ʈ ���ε�
		OnHitEventTriggered.AddDynamic(this, &ThisClass::ApplyCriticalHitDamage);
	}
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(GetController());

			// Ŭ���̾�Ʈ���� ���� ������, �׸��� ���� �÷��̾��� ��Ʈ�ѷ����� Ȯ��
			if (!HasAuthority() && PlayerController && PlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				UE_LOG(LogTemp, Error, TEXT("This code is running on the client for the local player."));

				// �ֱ������� ������ Ÿ�̸� ����
				SetGameTimer(GameTimer, GameTimerIndex++, [this]() { CheckOutOfSight(); }, 0.1f, true);
				SetGameTimer(GameTimer, GameTimerIndex++, [this]() { UpdateOverlayMaterial(); }, 0.1f, true);

				// �Է� ����ý��ۿ��� ���� ���ؽ�Ʈ �߰�
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
				{
					Subsystem->AddMappingContext(PlayerInputMappingContext, 0);
				}

				// ������ ���� �ʱ�ȭ
				PlayerController->InitializeItemShop();

				// ���� �� PostProcessVolume�� ã�� ĳ��
				for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
				{
					if (It->bUnbound)
					{
						UE_LOG(LogTemp, Log, TEXT("[AAOSCharacterBase::BeginPlay] Found PostProcessVolume."));
						PostProcessVolume = *It;
						break;
					}
				}
			}
		}));

	// �ִϸ��̼� �ν��Ͻ� ����
	UPlayerAnimInstance* PlayerAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(PlayerAnimInstance))
	{
		AnimInstance = PlayerAnimInstance;

		PlayerAnimInstance->OnEnableMoveNotifyBegin.BindUObject(this, &AAOSCharacterBase::EnableCharacterMove);
		PlayerAnimInstance->OnEnableSwitchActionNotifyBegin.BindUObject(this, &AAOSCharacterBase::EnableSwitchAction);
		PlayerAnimInstance->OnEnableGravityNotifyBegin.AddDynamic(this, &AAOSCharacterBase::EnableGravity);
		PlayerAnimInstance->OnDisableGravityNotifyBegin.AddDynamic(this, &AAOSCharacterBase::DisableGravity);
	}

	AOSGameState = Cast<AAOSGameState>(UGameplayStatics::GetGameState(this));
	if (HasAuthority() && ::IsValid(AOSGameState))
	{
		// �ʿ��� ���� ���� �ʱ�ȭ �ڵ� �߰�

	}
}

void AAOSCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// ��� ���� Ÿ�̸� ����
	for (auto& Elem : GameTimer)
	{
		GetWorld()->GetTimerManager().ClearTimer(Elem.Value);
	}
	GameTimer.Empty();

	// ��� �ɷ� Ÿ�̸� ����
	for (auto& Elem : AbilityTimer)
	{
		GetWorld()->GetTimerManager().ClearTimer(Elem.Value);
	}
	AbilityTimer.Empty();

	// ��� ���� ���� Ÿ�̸� ����
	for (auto& Elem : CrowdControlTimer)
	{
		GetWorld()->GetTimerManager().ClearTimer(Elem.Value);
	}
	CrowdControlTimer.Empty();
}

void AAOSCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AAOSCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (!GameInstance) return;

		if (ICharacterDataProviderInterface* Provider = Cast<ICharacterDataProviderInterface>(GameInstance->GetDataProvider(EObjectType::Player)))
		{
			StatComponent->InitStatComponent(Provider, ChampionName);
			AbilityStatComponent->InitAbilityStatComponent(Provider, StatComponent, ChampionName);

			StatComponent->OnHPNotEqualsMaxHP.AddDynamic(this, &AAOSCharacterBase::HPRegenEverySecond_Server);
			StatComponent->OnMPNotEqualsMaxMP.AddDynamic(this, &AAOSCharacterBase::MPRegenEverySecond_Server);
			StatComponent->OnHPEqualsMaxHP.AddDynamic(this, &AAOSCharacterBase::ClearHPRegenTimer_Server);
			StatComponent->OnMPEqualsMaxMP.AddDynamic(this, &AAOSCharacterBase::ClearMPRegenTimer_Server);

			StatComponent->OnCurrentLevelChanged.AddDynamic(AbilityStatComponent, &UAbilityStatComponent::UpdateLevelUpUI_Server);
		}
	}
}


void AAOSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
		{
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->SwitchInputMappingContext_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::OnCtrlKeyPressed);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->SwitchInputMappingContext_Action, ETriggerEvent::Triggered, this, &AAOSCharacterBase::OnCtrlKeyReleased);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_Ability_Q_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UpgradeAbility_Q_Server);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_Ability_E_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UpgradeAbility_E_Server);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_Ability_R_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UpgradeAbility_R_Server);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_Ability_LMB_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UpgradeAbility_LMB_Server);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_Ability_RMB_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UpgradeAbility_RMB_Server);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_1_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot1);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_2_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot2);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_3_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot3);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_4_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot4);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_5_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot5);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_6_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot6);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->ToggleItemShop_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ToggleItemShop);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Recall_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::Recall);
		}));
}

//==================== Ability Upgrade Functions ====================//

 /**
  * �ɷ��� ���׷��̵��ϴ� ���� ������ ó���մϴ�.
  * @param AbilityID ���׷��̵��� �ɷ��� ID�Դϴ�.
  * @param InitializeAbilityFunction �ɷ��� �ʱ�ȭ�ϴ� �Լ��Դϴ�.
  */
void AAOSCharacterBase::UpgradeAbility(EAbilityID AbilityID, TFunction<void(int32)> InitializeAbilityFunction)
{
	if (!IsValid(AOSPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UpgradeAbility_Server - AOSPlayerStateis null."));
		return;
	}

	if (AOSPlayerState->GetAbilityPoints() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAOSCharacterBase::UpgradeAbility_Server - Not enough ability points."));
		return;
	}

	if (!IsValid(AbilityStatComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UpgradeAbility_Server - AbilityStatComponentis null."));
		return;
	}

	FAbilityDetails Ability_StatTable = AbilityStatComponent->GetAbilityInfomation(AbilityID);

	// �ɷ��� ���׷��̵� �������� Ȯ�� �� ���� üũ
	if (!Ability_StatTable.bIsUpgradable || Ability_StatTable.CurrentLevel >= Ability_StatTable.MaxLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAOSCharacterBase::UpgradeAbility_Server - This ability cannot be upgraded or is already at the maximum level."));
		return;
	}

	// �ɷ� ���� ���׷��̵� �� ����Ʈ ����
	InitializeAbilityFunction(Ability_StatTable.CurrentLevel + 1);
	AOSPlayerState->SetAbilityPoints(AOSPlayerState->GetAbilityPoints() - 1);

	// UI ������Ʈ
	if (AOSPlayerState->GetAbilityPoints() > 0)
	{
		AbilityStatComponent->UpdateLevelUpUI_Server(0, StatComponent->GetCurrentLevel());
	}
	else
	{
		AbilityStatComponent->ToggleLevelUpUI_Server(false);
	}

	UE_LOG(LogTemp, Log, TEXT("AAOSCharacterBase::UpgradeAbility_Server - The ability has been successfully upgraded."));
}

// �� �ɷ¿� ���� ���׷��̵� �Լ�
void AAOSCharacterBase::UpgradeAbility_Q_Server_Implementation()
{
	UpgradeAbility(EAbilityID::Ability_Q, [&](int32 NewLevel) { AbilityStatComponent->InitializeAbility(EAbilityID::Ability_Q, NewLevel); });
}

void AAOSCharacterBase::UpgradeAbility_E_Server_Implementation()
{
	UpgradeAbility(EAbilityID::Ability_E, [&](int32 NewLevel) { AbilityStatComponent->InitializeAbility(EAbilityID::Ability_E, NewLevel); });
}

void AAOSCharacterBase::UpgradeAbility_R_Server_Implementation()
{
	UpgradeAbility(EAbilityID::Ability_R, [&](int32 NewLevel) { AbilityStatComponent->InitializeAbility(EAbilityID::Ability_R, NewLevel); });
}

void AAOSCharacterBase::UpgradeAbility_LMB_Server_Implementation()
{
	UpgradeAbility(EAbilityID::Ability_LMB, [&](int32 NewLevel) { AbilityStatComponent->InitializeAbility(EAbilityID::Ability_LMB, NewLevel); });
}

void AAOSCharacterBase::UpgradeAbility_RMB_Server_Implementation()
{
	UpgradeAbility(EAbilityID::Ability_RMB, [&](int32 NewLevel) { AbilityStatComponent->InitializeAbility(EAbilityID::Ability_RMB, NewLevel); });
}

//==================== Input Handling Functions ====================//

void AAOSCharacterBase::OnCtrlKeyPressed()
{
	HandleCtrlKeyInput(true);
}

void AAOSCharacterBase::OnCtrlKeyReleased()
{
	HandleCtrlKeyInput(false);
}

void AAOSCharacterBase::Recall()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	UAnimMontage* RecallMontage = GetOrLoadMontage("Recall", TEXT("/Game/ProjectAOS/Characters/Aurora/Animations/Recall_Montage.Recall_Montage"));
	if (!RecallMontage)
	{
		return;
	}

	ModifyCharacterState(ECharacterStateOperation::Remove, EBaseCharacterState::Move);
	ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::Recall);
	MovementComponent->StopMovementImmediately();
	
	PlayMontage_Server(RecallMontage, 1.0f, NAME_None);
}

void AAOSCharacterBase::UseItemSlot1_Implementation()
{
	if (!IsValid(AOSPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UseItemSlot1 - AOSPlayerStateis null."));
		return;
	}

	AOSPlayerState->UseItem(0);
}

void AAOSCharacterBase::UseItemSlot2_Implementation()
{
	if (!IsValid(AOSPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UseItemSlot1 - AOSPlayerStateis null."));
		return;
	}

	AOSPlayerState->UseItem(1);
}

void AAOSCharacterBase::UseItemSlot3_Implementation()
{
	if (!IsValid(AOSPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UseItemSlot1 - AOSPlayerStateis null."));
		return;
	}

	AOSPlayerState->UseItem(2);
}

void AAOSCharacterBase::UseItemSlot4_Implementation()
{
	if (!IsValid(AOSPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UseItemSlot1 - AOSPlayerStateis null."));
		return;
	}

	AOSPlayerState->UseItem(3);
}

void AAOSCharacterBase::UseItemSlot5_Implementation()
{
	if (!IsValid(AOSPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UseItemSlot1 - AOSPlayerStateis null."));
		return;
	}

	AOSPlayerState->UseItem(4);
}

void AAOSCharacterBase::UseItemSlot6_Implementation()
{
	if (!IsValid(AOSPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::UseItemSlot1 - AOSPlayerStateis null."));
		return;
	}

	AOSPlayerState->UseItem(5);
}

void AAOSCharacterBase::ToggleItemShop()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (AAOSPlayerController* AOSPC = Cast<AAOSPlayerController>(PC))
		{
			AOSPC->ToggleItemShopVisibility();
			bIsItemShopOpen = !bIsItemShopOpen;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to cast to AAOSPlayerController"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast to APlayerController"));
	}
}

/**
 * Ctrl Ű �Է� ó�� �Լ�
 * @param bPressed Ctrl Ű�� ���ȴ��� ����
 */
void AAOSCharacterBase::HandleCtrlKeyInput(bool bPressed)
{
	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(Controller);
	if (!IsValid(PlayerController))
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!IsValid(Subsystem))
	{
		return;
	}

	FModifyContextOptions ModifyContextOptions;
	ModifyContextOptions.bForceImmediately = true;
	ModifyContextOptions.bIgnoreAllPressedKeysUntilRelease = false;
	ModifyContextOptions.bNotifyUserSettings = false;

	if (bPressed)
	{
		Subsystem->RemoveMappingContext(PlayerInputMappingContext, ModifyContextOptions);
		Subsystem->AddMappingContext(PlayerCtrlInputMappingContext, 0, ModifyContextOptions);
		bCtrlKeyPressed = true;
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("AAOSCharacterBase::OnCtrlKeyPressed()"), true, true, FColor::Orange, 1.0f);
	}
	else
	{
		Subsystem->AddMappingContext(PlayerInputMappingContext, 0, ModifyContextOptions);
		Subsystem->RemoveMappingContext(PlayerCtrlInputMappingContext, ModifyContextOptions);
		bCtrlKeyPressed = false;
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("AAOSCharacterBase::OnCtrlKeyReleased()"), true, true, FColor::Yellow, 1.0f);
	}
}

//==================== Widget Functions ====================//

void AAOSCharacterBase::SetWidget(UUserWidgetBase* InUserWidgetBase)
{
	StateBar = Cast<UUW_StateBar>(InUserWidgetBase);

	if (::IsValid(StateBar))
	{
		StateBar->InitializeStateBar(StatComponent);
	}

	if (!HasAuthority() && IsLocallyControlled())
	{
		if (::IsValid(StateBar))
		{
			StateBar->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AAOSCharacterBase::SetWidgetVisibility(bool Visibility)
{
	Visibility ? StateBar->SetVisibility(ESlateVisibility::Visible) : StateBar->SetVisibility(ESlateVisibility::Hidden);
}

//==================== Sight Check Functions ====================//

void AAOSCharacterBase::CheckOutOfSight()
{
	if (!::IsValid(AOSGameState))
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSCharacterBase::CheckOutOfSight] AOSGameState is invalid."));
		return;
	}

	TArray<TObjectPtr<AAOSCharacterBase>> OpponentTeamPlayers =
		(TeamSide == ETeamSideBase::Blue) ? AOSGameState->GetRedTeamPlayers() : AOSGameState->GetBlueTeamPlayers();

	if (OpponentTeamPlayers.Num() == 0)
	{
		return;
	}

	const AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSCharacterBase::CheckOutOfSight] PlayerController is invalid."));
		return;
	}

	int32 ScreenWidth = 0;
	int32 ScreenHeight = 0;
	PlayerController->GetViewportSize(ScreenWidth, ScreenHeight);

	for (auto& Character : OpponentTeamPlayers)
	{
		if (!::IsValid(Character))
		{
			UE_LOG(LogTemp, Warning, TEXT("[AAOSCharacterBase::CheckOutOfSight] Character is invalid."));
			continue;
		}

		float Distance = FVector::Distance(GetActorLocation(), Character->GetActorLocation());
		if (Distance > 2000.f)
		{
			Character->SetWidgetVisibility(false);
			continue;
		}

		FVector2D ScreenLocation;
		if (PlayerController->ProjectWorldLocationToScreen(Character->GetActorLocation(), ScreenLocation))
		{
			int32 ScreenX = static_cast<int32>(ScreenLocation.X);
			int32 ScreenY = static_cast<int32>(ScreenLocation.Y);

			if (ScreenX > 0 && ScreenY > 0 && ScreenX < ScreenWidth && ScreenY < ScreenHeight)
			{
				//UE_LOG(LogTemp, Warning, TEXT("[AAOSCharacterBase::CheckOutOfSight] ScreenX: %d , ScreenY: %d || ScreenWidth: %d , ScreenHeight: %d"), ScreenX, ScreenY, ScreenWidth, ScreenHeight);

				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, false, this);
				bool bResult = GetWorld()->LineTraceSingleByChannel(
					HitResult,
					GetActorLocation(),
					Character->GetActorLocation(),
					ECC_Visibility,
					Params
				);

				if (bResult)
				{
					AActor* HitActor = HitResult.GetActor();
					if (::IsValid(HitActor))
					{
						if (HitActor == Character)
						{
							Character->SetWidgetVisibility(true);
						}
						else
						{
							Character->SetWidgetVisibility(false);
							UE_LOG(LogTemp, Warning, TEXT("[AAOSCharacterBase::CheckOutOfSight] Line trace hit an object other than the Character: %s"), *HitActor->GetName());
						}
					}
				}


#pragma region CollisionDebugDrawing
				FColor DrawColor = bResult ? FColor::Green : FColor::Red;
				float DebugLifeTime = 1.f;

				DrawDebugLine(
					GetWorld(),
					GetActorLocation(),
					Character->GetActorLocation(),
					DrawColor,
					false,
					DebugLifeTime
				);
#pragma endregion
			}
			else
			{
				Character->SetWidgetVisibility(false);
			}
		}
	}
}


//==================== Particle Functions ====================//

void AAOSCharacterBase::SpawnLevelUpParticle(int32 OldLevel, int32 NewLevel)
{
	if (OldLevel == NewLevel)
	{
		return;
	}

	UParticleSystem* Particle = GetOrLoadSharedParticle("LevelUp", TEXT("/Game/ParagonMinions/FX/Particles/SharedGameplay/States/LevelUp/P_LevelUp.P_LevelUp"));
	if (Particle)
	{
		LastCharacterLocation = GetActorLocation() - FVector(0, 0, 75.f);
		SpawnRootedParticleAtLocation_Server(Particle, FTransform(FRotator(1), LastCharacterLocation, FVector(1)));
	}
}

//==================== Character State Functions ====================//

void AAOSCharacterBase::UpdateCharacterState_Server_Implementation(EBaseCharacterState InCharacterState)
{
	CharacterState = InCharacterState;
}

void AAOSCharacterBase::UpdateInputValue_Server_Implementation(const float& InForwardInputValue, const float& InRightInputValue)
{
	ForwardInputValue = InForwardInputValue;
	RightInputValue = InRightInputValue;
}

void AAOSCharacterBase::UpdateAimValue_Server_Implementation(const float& InAimPitchValue, const float& InAimYawValue)
{
	CurrentAimYaw = InAimYawValue;
	CurrentAimPitch = InAimPitchValue;
}


//==================== HP/MP Regeneration Functions ====================//

void AAOSCharacterBase::HPRegenEverySecond_Server_Implementation()
{
	GetWorldTimerManager().ClearTimer(HPReganTimer);

	GetWorldTimerManager().SetTimer(
		HPReganTimer,
		FTimerDelegate::CreateLambda([this]()
			{
				if (!::IsValid(StatComponent))
				{
					UE_LOG(LogTemp, Error, TEXT("[HPRegenEverySecond_Server] StatComponentis null."));
					return;
				}

				float HPRegeneration = StatComponent->GetHealthRegeneration();
				StatComponent->ModifyCurrentHP(HPRegeneration);
			}),
		1.0f,
		true,
		1.0f
	);
}

void AAOSCharacterBase::MPRegenEverySecond_Server_Implementation()
{
	GetWorldTimerManager().ClearTimer(MPReganTimer);

	GetWorldTimerManager().SetTimer(
		MPReganTimer,
		FTimerDelegate::CreateLambda([this]()
			{
				if (!::IsValid(StatComponent))
				{
					UE_LOG(LogTemp, Error, TEXT("[MPRegenEverySecond_Server] StatComponentis null."));
					return;
				}
				float MPRegeneration = StatComponent->GetManaRegeneration();
				StatComponent->ModifyCurrentMP(MPRegeneration);
			}),
		1.0f,
		true,
		1.0f
	);
}

void AAOSCharacterBase::ClearHPRegenTimer_Server_Implementation()
{
	GetWorldTimerManager().ClearTimer(HPReganTimer);
}

void AAOSCharacterBase::ClearMPRegenTimer_Server_Implementation()
{
	GetWorldTimerManager().ClearTimer(MPReganTimer);
}

//==================== Character Attribute Functions ====================//

void AAOSCharacterBase::DecreaseHP_Server_Implementation()
{
	StatComponent->ModifyCurrentHP(-100.f);
}

void AAOSCharacterBase::DecreaseMP_Server_Implementation()
{
	StatComponent->ModifyCurrentMP(-100.f);
}

void AAOSCharacterBase::IncreaseEXP_Server_Implementation()
{
	StatComponent->ModifyCurrentEXP(20);
}

void AAOSCharacterBase::IncreaseLevel_Server_Implementation()
{
	StatComponent->SetCurrentLevel(StatComponent->GetCurrentLevel() + 1);
}

void AAOSCharacterBase::IncreaseCriticalChance_Server_Implementation()
{
	StatComponent->ModifyAccumulatedFlatCriticalChance(5);
}

void AAOSCharacterBase::IncreaseAttackSpeed_Server_Implementation()
{
	StatComponent->ModifyAccumulatedPercentAttackSpeed(50.f);
}

void AAOSCharacterBase::ChangeTeamSide_Server_Implementation(ETeamSideBase InTeamSide)
{
	TeamSide = InTeamSide;

	if (::IsValid(AOSGameState))
	{
		AOSGameState->RemovePlayerCharacter(this);
		AOSGameState->AddPlayerCharacter(this, TeamSide);
	}
}

//==================== Animation Functions ====================//

void AAOSCharacterBase::OnCharacterDeath()
{
	UE_LOG(LogTemp, Log, TEXT("[AAOSCharacterBase::OnCharacterDeath] Calling OnCharacterDeath Function."));

	bool bIsDeathInProgress = true;
	if (OnPreDeathEvent.IsBound())
	{
		OnPreDeathEvent.Broadcast(bIsDeathInProgress);
	}

	if (!bIsDeathInProgress)
	{
		return;
	}

	if (AAOSGameMode* GM = Cast<AAOSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		if (AAOSPlayerController* AOSPlayerController = Cast<AAOSPlayerController>(GetController()))
		{
			GM->RequestRespawn(AOSPlayerController);
		}
	}

	UParticleSystem* DeathParticle = nullptr;
	if (UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		const FSharedGameplay* SharedGamePlayDataTable = GameInstance->GetSharedGamePlayParticles();
		if (!SharedGamePlayDataTable)
		{
			DeathParticle = Cast<UParticleSystem>(StaticLoadObject(UParticleSystem::StaticClass(), NULL, TEXT("/Game/ParagonMinions/FX/Particles/SharedGameplay/States/Death/FX/P_Death_Buff.P_Death_Buff")));
		}
		else
		{
			DeathParticle = SharedGamePlayDataTable->GetSharedGamePlayParticle(TEXT("Death_Default"));
		}
	}

	if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath))
	{
		StatComponent->OnOutOfCurrentHP.RemoveDynamic(this, &ThisClass::OnCharacterDeath);
	}

	if (StatComponent->OnHPEqualsMaxHP.IsAlreadyBound(this, &AAOSCharacterBase::HPRegenEverySecond_Server))
	{
		StatComponent->OnHPEqualsMaxHP.RemoveDynamic(this, &AAOSCharacterBase::HPRegenEverySecond_Server);
	}

	if (StatComponent->OnMPEqualsMaxMP.IsAlreadyBound(this, &AAOSCharacterBase::MPRegenEverySecond_Server))
	{
		StatComponent->OnMPEqualsMaxMP.RemoveDynamic(this, &AAOSCharacterBase::MPRegenEverySecond_Server);
	}

	GetWorldTimerManager().ClearTimer(HPReganTimer);
	GetWorldTimerManager().ClearTimer(MPReganTimer);

	EnumAddFlags(CharacterState, EBaseCharacterState::Death);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	//SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	ActivatePostProcessEffect_Client();

	UAnimMontage* Montage = GetOrLoadMontage("Death", TEXT(""));
	if (PlayAnimMontage(Montage) <= 0) SetActorHiddenInGame(true);
	if (DeathParticle) SpawnRootedParticleAtLocation_Server(DeathParticle, FTransform(FRotator(0), GetActorLocation(), FVector(1)));

}

void AAOSCharacterBase::ActivatePostProcessEffect_Client_Implementation()
{
	/*CameraComponent->PostProcessSettings.ColorSaturation = FVector4(0, 0, 0, 1);
	CameraComponent->PostProcessSettings.ColorGamma = FVector4(1, 1, 1, 0.8f);*/

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (PostProcessVolume)
	{
		UE_LOG(LogTemp, Log, TEXT("[AAOSCharacterBase::ActivatePostProcessEffect_Client] Calling ActivatePostProcessEffect Function."));

		// ColorSaturation�� ColorGamma ���� �����մϴ�.
		PostProcessVolume->Settings.bOverride_ColorSaturation = true;
		PostProcessVolume->Settings.ColorSaturation = FVector4(0, 0, 0, 1); // ������� ����

		PostProcessVolume->Settings.bOverride_ColorGamma = true;
		PostProcessVolume->Settings.ColorGamma = FVector4(1.0f, 1.0f, 1.0f, 0.8f); // Gamma ���� ����
	}
	else
	{
		// ���� �� PostProcessVolume�� ã���ϴ�.
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
		{
			if (It->bUnbound)
			{
				UE_LOG(LogTemp, Log, TEXT("[AAOSCharacterBase::ActivatePostProcessEffect_Client] Found PostProcessVolume."));

				PostProcessVolume = *It;
				ActivatePostProcessEffect_Client();
				break;
			}
		}
	}
}

void AAOSCharacterBase::DeActivatePostProcessEffect_Client_Implementation()
{
	/*CameraComponent->PostProcessSettings.ColorSaturation = FVector4(1, 1, 1, 1);
	CameraComponent->PostProcessSettings.ColorGamma = FVector4(1, 1, 1, 1);*/

	if (PostProcessVolume)
	{
		UE_LOG(LogTemp, Log, TEXT("[AAOSCharacterBase::DeActivatePostProcessEffect_Client] Calling DeActivatePostProcessEffect Function."));

		// ColorSaturation�� ColorGamma ���� �����մϴ�.
		PostProcessVolume->Settings.bOverride_ColorSaturation = true;
		PostProcessVolume->Settings.ColorSaturation = FVector4(1, 1, 1, 1);

		PostProcessVolume->Settings.bOverride_ColorGamma = true;
		PostProcessVolume->Settings.ColorGamma = FVector4(1.0f, 1.0f, 1.0f, 1.0f); 
	}

	while (!DamageWidgetQueue.IsEmpty())
	{
		UWidgetComponent* DamageWidgetComponent = nullptr;

		// ť���� ���� ������Ʈ�� ���� ����
		if (DamageWidgetQueue.Dequeue(DamageWidgetComponent))
		{
			if (DamageWidgetComponent)
			{
				DamageWidgetComponent->DestroyComponent();
			}
		}
	}

	DamageWidgetQueue.Empty();
}

void AAOSCharacterBase::Respawn()
{
	UE_LOG(LogTemp, Log, TEXT("[AAOSCharacterBase::Respawn] Calling Respawn Function."));

	EnumRemoveFlags(CharacterState, EBaseCharacterState::Death);

	GetCapsuleComponent()->SetCollisionProfileName(FName("AOSCharacter"));
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == false)
	{
		StatComponent->OnOutOfCurrentHP.AddDynamic(this, &ThisClass::OnCharacterDeath);
	}

	StatComponent->OnHPNotEqualsMaxHP.AddDynamic(this, &AAOSCharacterBase::HPRegenEverySecond_Server);
	StatComponent->OnMPNotEqualsMaxMP.AddDynamic(this, &AAOSCharacterBase::MPRegenEverySecond_Server);

	StatComponent->ModifyCurrentHP(StatComponent->GetMaxHP());
	StatComponent->ModifyCurrentMP(StatComponent->GetMaxMP());

	DeActivatePostProcessEffect_Client();
}

//==================== Character Control Functions ====================//

void AAOSCharacterBase::EnableCharacterMove()
{
	ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::Move);
}

void AAOSCharacterBase::EnableSwitchAction()
{
	ModifyCharacterState(ECharacterStateOperation::Add, EBaseCharacterState::SwitchAction);
}


void AAOSCharacterBase::EnableUseControllerRotationYaw()
{
	bUseControllerRotationYaw = true;
}

void AAOSCharacterBase::EnableGravity()
{
	UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement();
	if (!CharacterMovementComp)
	{
		return;
	}

	CharacterMovementComp->GravityScale = 1.0;
}

void AAOSCharacterBase::DisableGravity()
{
	UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement();
	if (!CharacterMovementComp)
	{
		return;
	}

	CharacterMovementComp->GravityScale = 0;
}

//==================== Utility Functions ====================//

const FName AAOSCharacterBase::GetAttackMontageSection(const int32& Section)
{
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}


void AAOSCharacterBase::SaveCharacterTransform()
{
	// ���� ĳ������ ��ġ, ȸ��, ���� ���͸� ����
	LastCharacterLocation = GetActorLocation() - FVector(0, 0, 95.f);
	LastCharacterRotation = GetActorRotation();
	LastForwardVector = GetActorForwardVector();
	LastRightVector = GetActorRightVector();
	LastUpVector = GetActorUpVector();
}

void AAOSCharacterBase::SetGameTimer(TMap<int32, FTimerHandle>& Timers, int32 TimerID, TFunction<void()> Callback, float Duration, bool bLoop, float FirstDelay)
{
	FTimerHandle& TimerHandle = Timers.FindOrAdd(TimerID);
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda(Callback);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, bLoop, FirstDelay);
	UE_LOG(LogTemp, Log, TEXT("SetGameTimer called with TimerID: %d, Duration: %f, Loop: %s"), TimerID, Duration, bLoop ? TEXT("true") : TEXT("false"));
}

void AAOSCharacterBase::ClearGameTimer(TMap<int32, FTimerHandle>& Timers, int32 TimerID)
{
	FTimerHandle* TimerHandle = Timers.Find(TimerID);
	if (TimerHandle)
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
		Timers.Remove(TimerID);
	}
}

bool AAOSCharacterBase::IsGameTimerActive(TMap<int32, FTimerHandle>& Timers, const int32 ItemID) const
{
	const FTimerHandle* TimerHandle = Timers.Find(ItemID);
	return TimerHandle && GetWorld()->GetTimerManager().IsTimerActive(*TimerHandle);
}


void AAOSCharacterBase::OnRep_CharacterStateChanged()
{
	Super::OnRep_CharacterStateChanged();

}

void AAOSCharacterBase::ServerNotifyAbilityUse_Implementation(EAbilityID AbilityID, ETriggerEvent TriggerEvent)
{
	OnAbilityUse(AbilityID, TriggerEvent);
}

void AAOSCharacterBase::OnAbilityUse(EAbilityID AbilityID, ETriggerEvent TriggerEvent)
{
}


void AAOSCharacterBase::DisableJump()
{
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->RemoveActionBinding("Jump", EInputEvent::IE_Pressed);
	}
}

void AAOSCharacterBase::EnableJump()
{
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &AAOSCharacterBase::Jump);
	}
}

/**
 * ī�޶��� ���� ���Ϳ� ���� ������ ������� �浹 ������ ����մϴ�.
 * �� �Լ��� ī�޶� ��ġ���� ���� ������ ���ǵ� �������� ���� Ʈ���̽��� �����մϴ�.
 * ���� ���� Ʈ���̽��� ��ü�� �浹�ϸ�, �浹 ������ �浹 ��ġ�� ������Ʈ�մϴ�.
 *
 * @param TraceRange ������ �浹�� Ȯ���ؾ� �ϴ� �ִ� �Ÿ��Դϴ�.
 * @return �浹 ���ο� ���� �浹 ������ ������ FImpactResult ����ü�Դϴ�.
 */
FImpactResult AAOSCharacterBase::GetImpactPoint(const float TraceRange)
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	FImpactResult ImpactResult;

	if (IsValid(CameraManager) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraManager is null."));
		return ImpactResult;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation();
	FVector EndPoint = CameraLocation + CameraManager->GetActorForwardVector() * (TraceRange > 0 ? TraceRange : 1000.f);

	FCollisionQueryParams params(NAME_None, false, this);
	bool bResult = GetWorld()->LineTraceSingleByChannel(
		ImpactResult.HitResult,
		CameraLocation,
		EndPoint,
		ECollisionChannel::ECC_Visibility,
		params
	);

	ImpactResult.bHit = bResult;
	ImpactResult.ImpactPoint = bResult ? ImpactResult.HitResult.Location : EndPoint;

	DrawDebugSphere(GetWorld(), ImpactResult.ImpactPoint, 10.0f, 12, bResult ? FColor::Red : FColor::Green, false, -1.0f);

	return ImpactResult;
}

FImpactResult AAOSCharacterBase::GetSweepImpactPoint(const float TraceRange)
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	FImpactResult ImpactResult;

	if (!IsValid(CameraManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraManager is null."));
		return ImpactResult;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation();
	FVector EndPoint = CameraLocation + CameraManager->GetActorForwardVector() * (TraceRange > 0 ? TraceRange : 1000.f);

	FCollisionQueryParams Params(NAME_None, false, this);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(50.0f);  // ���� ������ ������

	bool bResult = GetWorld()->SweepSingleByChannel(
		ImpactResult.HitResult,
		CameraLocation,
		EndPoint,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		CollisionShape,
		Params
	);

	ImpactResult.bHit = bResult;
	ImpactResult.ImpactPoint = bResult ? ImpactResult.HitResult.Location : EndPoint;

	DrawDebugSphere(GetWorld(), ImpactResult.ImpactPoint, 50.0f, 12, bResult ? FColor::Red : FColor::Green, false, -1.0f);

	return ImpactResult;
}

void AAOSCharacterBase::UpdateOverlayMaterial()
{
	const float BasicAttackRange = AbilityStatComponent->GetAbilityStatTable(EAbilityID::Ability_LMB).Range;

	// �� Ŭ���̾�Ʈ���� �浹 ������ ����մϴ�.
	FImpactResult ImpactResult = GetSweepImpactPoint(BasicAttackRange);

	// ���� Ÿ���� �������� ���׸����� �ʱ�ȭ�մϴ�.
	if (CurrentTarget)
	{
		UMeshComponent* MeshComponent = Cast<UMeshComponent>(CurrentTarget->GetComponentByClass(UMeshComponent::StaticClass()));
		if (MeshComponent)
		{
			MeshComponent->SetOverlayMaterial(OriginalMaterial); // ���� ���׸���� ����
		}

		CurrentTarget = nullptr;
	}

	// �浹�� ��ü�� ��ȿ���� Ȯ���մϴ�.
	if (ImpactResult.bHit && ::IsValid(ImpactResult.HitResult.GetActor()))
	{
		// �浹�� ��ü�� �浹 ä���� Ȯ���մϴ�.
		ECollisionChannel HitChannel = ImpactResult.HitResult.GetComponent()->GetCollisionObjectType();
		if (HitChannel == ECC_WorldStatic || HitChannel == ECC_WorldDynamic)
		{
		}
		else
		{
			CurrentTarget = ImpactResult.HitResult.GetActor();

			UMeshComponent* MeshComponent = Cast<UMeshComponent>(CurrentTarget->GetComponentByClass(UMeshComponent::StaticClass()));
			if (MeshComponent && OverlayMaterial_Ally && OverlayMaterial_Enemy)
			{
				// ���� ���׸����� �����մϴ�.
				OriginalMaterial = MeshComponent->GetOverlayMaterial();

				ACharacterBase* Character = Cast<ACharacterBase>(CurrentTarget);
				if (Character)
				{
					if (this->TeamSide != Character->TeamSide)
					{
						MeshComponent->SetOverlayMaterial(OverlayMaterial_Enemy);
					}
					else
					{
						MeshComponent->SetOverlayMaterial(OverlayMaterial_Ally);
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Mesh component or overlay material is invalid on target: %s"), *CurrentTarget->GetName());
			}
		}
	}
}