// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/AOSCharacterBase.h"


// Game ���� ��� ����
#include "Game/AOSGameMode.h"
#include "Game/AOSGameState.h"
#include "Game/AOSPlayerState.h"

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
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->GravityScale = 1.6f;
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

	if (HasAuthority() == true)
	{
		GetCharacterMovement()->SetIsReplicated(true);
	}

	Index = -1;
	CharacterState = EBaseCharacterState::Basic;
	ObjectType = EObjectType::Player;

	TotalAttacks = 0;
	CriticalHits = 0;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("AOSCharacter"));

	SetReplicates(true);

	bIsCtrlKeyPressed = false;
	PrimaryActorTick.bCanEverTick = true;
}

void AAOSCharacterBase::InitializeAbilityParticles()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> LEVELUP(TEXT("/Game/ParagonMinions/FX/Particles/SharedGameplay/States/LevelUp/P_LevelUp.P_LevelUp"));
	if (LEVELUP.Succeeded()) LevelUpParticle = LEVELUP.Object;
}

void AAOSCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() && IsLocallyControlled())
	{
		if (PreviousCharacterState != CharacterState)
		{
			UpdateCharacterState_Server(CharacterState);
			PreviousCharacterState = CharacterState;
		}

		if (PreviousForwardInputValue != ForwardInputValue || PreviousRightInputValue != RightInputValue)
		{
			UpdateInputValue_Server(ForwardInputValue, RightInputValue);
		}
	}
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

						if (AController* Controller = GetController())
						{
							AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(Controller);
							if (::IsValid(PlayerController))
							{
								PlayerController->InitializeHUD(SelectedCharacterIndex);
								PlayerController->InitializeItemShop();
							}
						}
					}
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

	// Ŭ���̾�Ʈ���� ����
	if (!HasAuthority() && GetOwner() == UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		GetWorld()->GetTimerManager().SetTimer(DistanceCheckTimerHandle, this, &ThisClass::CheckOutOfSight, 0.1f, true);

		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
			{
				if (GetController())
				{
					AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(GetController());
					if (::IsValid(PlayerController))
					{
						UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
						if (::IsValid(Subsystem))
						{
							Subsystem->AddMappingContext(PlayerInputMappingContext, 0);
						}

					}
				}
			}
		));


		// ���� �� PostProcessVolume�� ã���ϴ�.
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

	// �ִϸ��̼� �ν��Ͻ� ����
	AnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(AnimInstance))
	{
		if (!HasAuthority() && GetOwner() == UGameplayStatics::GetPlayerCharacter(this, 0))
		{
			AnimInstance->OnEnableMoveNotifyBegin.BindUObject(this, &AAOSCharacterBase::EnableCharacterMove);
			AnimInstance->OnEnableSwithActionNotifyBegin.BindUObject(this, &AAOSCharacterBase::EnableSwitchAtion);
		}
	}

	// ���� ���� ����
	AOSGameState = Cast<AAOSGameState>(UGameplayStatics::GetGameState(this));
	if (HasAuthority() && ::IsValid(AOSGameState))
	{
		// �ʿ��� ���� ���� �ʱ�ȭ �ڵ� �߰�

	}

	// ĳ���� �̵� �ӵ� ����
	ChangeMovementSpeed(0, StatComponent->GetMovementSpeed());
}

void AAOSCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(DistanceCheckTimerHandle);
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
		AbilityStatComponent->InitAbilityStatComponent(StatComponent, SelectedCharacterIndex);
		StatComponent->InitializeStatComponent(SelectedCharacterIndex);

		StatComponent->OnHPNotEqualsMaxHP.AddDynamic(this, &AAOSCharacterBase::HPRegenEverySecond_Server);
		StatComponent->OnMPNotEqualsMaxMP.AddDynamic(this, &AAOSCharacterBase::MPRegenEverySecond_Server);
		StatComponent->OnHPEqualsMaxHP.AddDynamic(this, &AAOSCharacterBase::ClearHPRegenTimer_Server);
		StatComponent->OnMPEqualsMaxMP.AddDynamic(this, &AAOSCharacterBase::ClearMPRegenTimer_Server);
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

	FAbilityAttributes Ability_StatTable = AbilityStatComponent->GetAbilityInfomation(AbilityID);

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

//==================== Replication Functions ====================//

void AAOSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME_CONDITION(ThisClass, CharacterState, COND_SkipOwner);
	DOREPLIFETIME(ThisClass, ForwardInputValue);
	DOREPLIFETIME(ThisClass, RightInputValue);
	DOREPLIFETIME(ThisClass, CurrentAimPitch);
	DOREPLIFETIME(ThisClass, CurrentAimYaw);
	DOREPLIFETIME(ThisClass, CrowdControl);
	DOREPLIFETIME(ThisClass, bIsDead);
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





//==================== Attack and Damage Functions ====================//

/*
	ũ��Ƽ�� Ȯ���� �� ��Ȯ�ϰ� �ݿ��ϱ� ����, Ư�� Ƚ�� �ȿ� ������ Ȯ���� �ش��ϴ� Ƚ����ŭ ũ��Ƽ���� �߻��ϵ��� �����մϴ�.
	 - �⺻ Ȯ�� ����		: ũ��Ƽ�� Ȯ���� �⺻������ �������� ��������, ���� ������ �߰��Ͽ� ũ��Ƽ���� ������ Ȯ���� �°� �߻��ϵ��� �����մϴ�.
	 - Ȯ�� ���� �� ����	: ������ ������ ���� Ȯ���� ������Ű��, �����ϸ� �ʱ�ȭ�Ͽ� ���� Ȯ���� �������� �մϴ�.
*/
void AAOSCharacterBase::ApplyCriticalHitDamage(FDamageInfomation& DamageInfomation)
{
	// �⺻ ũ��Ƽ�� Ȯ�� (��: 0.25)
	float CriticalChance = static_cast<float>(StatComponent->GetCriticalChance()) / 100;
	if (CriticalChance <= 0)
	{
		return;
	}

	// ���� ���ݿ� ���� ũ��Ƽ�� Ȯ���� ����մϴ�.
	float ExpectedCriticalHits = TotalAttacks * CriticalChance;
	float ActualCriticalHits = static_cast<float>(CriticalHits);

	// ���� Ȯ���� ����մϴ�.
	float Adjustment = (ExpectedCriticalHits - ActualCriticalHits) / (TotalAttacks + 1);
	float AdjustedCriticalChance = CriticalChance + Adjustment;

	// ������ ���� �����Ͽ� ũ��Ƽ�� ��Ʈ ���θ� �����մϴ�.
	bool bIsCriticalHit = (FMath::FRand() <= AdjustedCriticalChance);

	if (bIsCriticalHit)
	{
		CriticalHits++;

		if (::IsValid(StatComponent))
		{
			float AttackDamage = StatComponent->GetAttackDamage();
			DamageInfomation.AddDamage(EDamageType::Critical, AttackDamage * 0.7);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::ApplyCriticalHitDamage - StatComponent is no vaild."));
		}
	}
	TotalAttacks++;
}

/**
 * ApplyDamage_Server_Implementation
 *
 * �� �Լ��� �������� ȣ��Ǹ�, ������ �� ĳ���Ϳ��� �������� �����մϴ�.
 * Ability_LMB�� ��� �⺻ ���� �������� ���Ǹ�, �ٸ� �ɷ��� ��ų �������� ���˴ϴ�.
 * ����ε� ���¿����� �⺻ ������ �����մϴ�.
 * ���� ��ȿ�ϸ� �ش� ������ ��Ʈ�� ���� �̺�Ʈ�� Ʈ�����ϰ�, ������ �������� �����մϴ�.
 *
 * @param Enemy �������� ���� �� ĳ�����Դϴ�.
 * @param DamageInfomation ������ ���� ����ü�Դϴ�.
 * @param EventInstigator ������ �ν�Ƽ������(�ַ� �÷��̾� ��Ʈ�ѷ�)�Դϴ�.
 * @param DamageCauser �������� ����Ų �����Դϴ�.
 */
void AAOSCharacterBase::ApplyDamage_Server_Implementation(ACharacterBase* Enemy, FDamageInfomation DamageInfomation, AController* EventInstigator, AActor* DamageCauser)
{
	if (!::IsValid(Enemy))
	{
		return;
	}

	if (DamageInfomation.AbilityID == EAbilityID::Ability_LMB)
	{
		// �Ǹ� ������ ��� �⺻ ������ ����
		if (EnumHasAnyFlags(CrowdControl, ECrowdControlBase::Blind))
		{
			// To do: ����ε� ���� ó��
			return;
		}
	}

	bool bResult = Enemy->ValidateHit(DamageInfomation.AbilityID);
	if (!bResult)
	{
		return;
	}

	// OnHit �̺�Ʈ ó��
	if (EnumHasAnyFlags(DamageInfomation.AttackEffect, EAttackEffect::OnHit) && OnHitEventTriggered.IsBound())
	{
		OnHitEventTriggered.Broadcast(DamageInfomation);
	}

	// OnAttack �̺�Ʈ ó��
	if (EnumHasAnyFlags(DamageInfomation.AttackEffect, EAttackEffect::OnAttack) && OnAttackEventTriggered.IsBound())
	{
		OnAttackEventTriggered.Broadcast(DamageInfomation);
	}

	// AbilityEffects �̺�Ʈ ó��
	if (EnumHasAnyFlags(DamageInfomation.AttackEffect, EAttackEffect::AbilityEffects) && OnAbilityEffectsEventTriggered.IsBound())
	{
		OnAbilityEffectsEventTriggered.Broadcast(DamageInfomation);
	}

	Enemy->ReceiveDamage(DamageInfomation, EventInstigator, DamageCauser);
}

//==================== Crowd Control Functions ====================//

/**
 * GetCrowdControl
 *
 * �� �Լ��� ������ ���� ����(Crowd Control) ������ �����մϴ�.
 * �� ���� ���� ������ �̵� �ӵ�, ���� �ӵ�, ��ų ���, �þ� � ������ ��Ĩ�ϴ�.
 * ����� ���� ���� ������ ���� �ð��� ���� �� �ڵ����� �����˴ϴ�.
 *
 * @param InCondition ������ ���� ���� �����Դϴ�.
 * @param InDuration ���� ��� ����Ǵ� ���� �ð��Դϴ�.
 * @param InPercent ���� ������ ȿ�� ������ ��Ÿ���� ������Դϴ�. (0.0 ~ 1.0)
 */
void AAOSCharacterBase::GetCrowdControl(ECrowdControlBase InCondition, float InDuration, float InPercent)
{
	float Percent = FMath::Clamp<float>(InPercent, 0.0f, 1.0f);

	switch (InCondition)
	{
	case ECrowdControlBase::None:
		break;

	case ECrowdControlBase::Slow: // �̵��ӵ� ��ȭ
		ApplyMovementSpeedDebuff(Percent, InDuration);
		break;

	case ECrowdControlBase::Cripple: // ���ݼӵ� ��ȭ
		ApplyAttackSpeedDebuff(Percent, InDuration);
		break;

	case ECrowdControlBase::Silence: // ħ�� (��ų ��� �Ұ�)
		ApplySilenceDebuff(InDuration);
		break;

	case ECrowdControlBase::Blind: // �Ǹ� (�⺻ ���� ������)
		ApplyBlindDebuff(InDuration);
		break;

	case ECrowdControlBase::BlockedSight: // �þ� ����
		break;

	case ECrowdControlBase::Snare: // �ӹ�
		break;

	case ECrowdControlBase::Stun: // ����
		ApplyStunDebuff(InDuration);
		break;

	case ECrowdControlBase::Taunt: // ����
		break;

	default:
		break;
	}
}

void AAOSCharacterBase::ApplyMovementSpeedDebuff(float Percent, float Duration)
{
	LastMovementSpeed = StatComponent->GetMovementSpeed();
	ChangeMovementSpeed(0, LastMovementSpeed * (1 - Percent));
	EnumAddFlags(CrowdControl, ECrowdControlBase::Slow);

	GetWorld()->GetTimerManager().SetTimer(
		CrowdControlTimer,
		FTimerDelegate::CreateLambda([this]()
			{
				ChangeMovementSpeed(0, LastMovementSpeed);
				EnumRemoveFlags(CrowdControl, ECrowdControlBase::Slow);
			}),
		Duration,
		false
	);
}

void AAOSCharacterBase::ApplyAttackSpeedDebuff(float Percent, float Duration)
{
	LastAttackSpeed = StatComponent->GetAttackSpeed();
	StatComponent->SetAttackSpeed(LastAttackSpeed * (1 - Percent));
	EnumAddFlags(CrowdControl, ECrowdControlBase::Cripple);

	GetWorld()->GetTimerManager().SetTimer(
		CrowdControlTimer,
		FTimerDelegate::CreateLambda([this]()
			{
				StatComponent->SetAttackSpeed(LastAttackSpeed);
				EnumRemoveFlags(CrowdControl, ECrowdControlBase::Cripple);
			}),
		Duration,
		false
	);
}

void AAOSCharacterBase::ApplySilenceDebuff(float Duration)
{
	AbilityStatComponent->BanUseAbilityFewSeconds(Duration);
	EnumAddFlags(CrowdControl, ECrowdControlBase::Silence);

	GetWorld()->GetTimerManager().SetTimer(
		CrowdControlTimer,
		FTimerDelegate::CreateLambda([this]()
			{
				EnumRemoveFlags(CrowdControl, ECrowdControlBase::Silence);
			}),
		Duration,
		false
	);
}

void AAOSCharacterBase::ApplyBlindDebuff(float Duration)
{
	EnumAddFlags(CrowdControl, ECrowdControlBase::Blind);

	GetWorld()->GetTimerManager().SetTimer(
		CrowdControlTimer,
		FTimerDelegate::CreateLambda([this]()
			{
				EnumRemoveFlags(CrowdControl, ECrowdControlBase::Blind);
			}),
		Duration,
		false
	);
}

void AAOSCharacterBase::ApplyStunDebuff(float Duration)
{
	if (::IsValid(AnimInstance))
	{
		AnimInstance->StopAllMontages(0.1f);
		AnimInstance->PlayMontage(Stun_Montage, 1.0f);
	}
	AbilityStatComponent->BanUseAbilityFewSeconds(Duration);
	EnumRemoveFlags(CharacterState, EBaseCharacterState::Move);

	GetWorld()->GetTimerManager().SetTimer(
		CrowdControlTimer,
		FTimerDelegate::CreateLambda([this]()
			{
				EnumAddFlags(CharacterState, EBaseCharacterState::Move);
				if (::IsValid(AnimInstance))
				{
					AnimInstance->StopAllMontages(0.3f);
				}
			}),
		Duration,
		false
	);
}

void AAOSCharacterBase::ApplayCrowdControl_Server_Implementation(ACharacterBase* Enemy, ECrowdControlBase InCondition, float InDuration, float InPercent)
{
	if (HasAuthority())
	{
		ApplayCrowdControl_NetMulticast(Enemy, InCondition, InDuration, InPercent);
	}
}

void AAOSCharacterBase::ApplayCrowdControl_NetMulticast_Implementation(ACharacterBase* Enemy, ECrowdControlBase InCondition, float InDuration, float InPercent)
{
	if (::IsValid(Enemy))
	{
		Enemy->GetCrowdControl(InCondition, InDuration, InPercent);
	}
}

//==================== Particle Functions ====================//

void AAOSCharacterBase::SpawnRootedParicleAtLocation_Server_Implementation(UParticleSystem* Particle, FTransform Transform)
{
	if (HasAuthority())
	{
		SpawnRootedParicleAtLocation_Multicast(Particle, Transform);
	}
}

void AAOSCharacterBase::SpawnRootedParicleAtLocation_Multicast_Implementation(UParticleSystem* Particle, FTransform Transform)
{
	if (!HasAuthority())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), true, EPSCPoolMethod::None, true);
	}
}

void AAOSCharacterBase::SpawnAttachedParicleAtLocation_Server_Implementation(UParticleSystem* Particle, FTransform Transform)
{
	if (HasAuthority())
	{
		SpawnAttachedParicleAtLocation_Multicast(Particle, Transform);
	}
}

void AAOSCharacterBase::SpawnAttachedParicleAtLocation_Multicast_Implementation(UParticleSystem* Particle, FTransform Transform)
{
	if (!HasAuthority())
	{
		UGameplayStatics::SpawnEmitterAttached(
			Particle, GetMesh(), "Name_None", Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), EAttachLocation::KeepWorldPosition, true, EPSCPoolMethod::None, true
		);
	}
}

void AAOSCharacterBase::SpawnCircularParicleAtLocation_Server_Implementation(UParticleSystem* Particle, FCircularParticleInfomation CircularParticleInfomation, FTransform InTransform)
{
	if (HasAuthority())
	{
		SpawnCircularParicleAtLocation_Multicast(Particle, CircularParticleInfomation, InTransform);
	}
}
void AAOSCharacterBase::SpawnCircularParicleAtLocation_Multicast_Implementation(UParticleSystem* Particle, FCircularParticleInfomation CircularParticleInfomation, FTransform InTransform)
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		if (!Particle || !IsValid(Particle))
		{
			UE_LOG(LogTemp, Error, TEXT("[AAOSCharacterBase::SpawnCircularParicleAtLocation_Multicast] ParticleSystem is null"));
			return;
		}

		FCircularParticleInfomation NewParticle;
		FTimerHandle NewTimereHandle;

		NewParticle = CircularParticleInfomation;
		NewParticle.Iterator = 0;
		NewParticle.ParticleID = ParticleInfoList.Num(); // ���� ID�� �迭 ũ��� ����
		NewParticle.Particle = Particle;
		NewParticle.Transform = InTransform;

		TimerHandleList.Add(NewTimereHandle);
		ParticleInfoList.Add(NewParticle);

		int32 ParticleID = NewParticle.ParticleID;

		// ��� �ڵ�: TimerHandleList�� ParticleInfoList�� ũ�� Ȯ��
		if (TimerHandleList.Num() != ParticleInfoList.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("TimerHandleList and ParticleInfoList size mismatch."));
			return;
		}

		GetWorldTimerManager().SetTimer(
			TimerHandleList[ParticleID],
			[this, ParticleID]()
			{
				if (!ParticleInfoList.IsValidIndex(ParticleID) || !TimerHandleList.IsValidIndex(ParticleID))
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid ParticleID: %d"), ParticleID);

					if (TimerHandleList.IsValidIndex(ParticleID))
					{
						GetWorldTimerManager().ClearTimer(TimerHandleList[ParticleID]);
						TimerHandleList.RemoveAt(ParticleID);
					}
					if (ParticleInfoList.IsValidIndex(ParticleID))
					{
						ParticleInfoList.RemoveAt(ParticleID);
					}

					return;
				}

				FCircularParticleInfomation& Info = ParticleInfoList[ParticleID];

				if (!Info.Particle || !Info.IsValid())
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid Particle in ParticleInfoList at ID: %d"), ParticleID);
					return;
				}

				const FVector Location = Info.Transform.GetLocation();
				const FVector Scale = Info.Transform.GetScale3D();
				const FVector ForwardVector = Info.ForwardVector;
				const FVector UpVector = Info.UpVector;

				const int32 Iterator = Info.Iterator;
				const int32 Particles = Info.NumOfParicles;

				const float Angle = Info.Angle;
				const float AngleRad = FMath::DegreesToRadians(Iterator * Angle);
				const float Radius = Info.Radius;
				const float Lifetime = Info.Lifetime;
				const float Rate = Info.Rate;

				FVector SpawnLocation = Location + ForwardVector.RotateAngleAxis(Iterator * Angle, UpVector) * Radius;
				FRotator SpawnRotation = ForwardVector.Rotation();
				SpawnRotation.Yaw += Iterator * Angle + 90;

				UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Info.Particle, SpawnLocation, SpawnRotation, Scale, false, EPSCPoolMethod::AutoRelease, false);
				if (PSC)
				{
					//UE_LOG(LogTemp, Warning, TEXT("[AOSCharacterBase::SpawnCircularParicleAtLocation] Spawn rooted particle %d %f"), Iterator, (Particles - Iterator - 1) * Rate + Lifetime);
					PSC->SetFloatParameter(FName("AbilityDuration"), (Particles - Iterator - 1) * Rate + Lifetime);
					PSC->SetFloatParameter(FName("DurationRange"), (Particles - Iterator - 1) * Rate + Lifetime);
					PSC->Activate();
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to spawn particle system at ID: %d"), ParticleID);
				}

				Info.Iterator++;

				if (Info.Iterator >= Particles)
				{
					if (TimerHandleList.IsValidIndex(ParticleID))
					{
						GetWorldTimerManager().ClearTimer(TimerHandleList[ParticleID]);
						TimerHandleList.RemoveAt(ParticleID);
					}
					if (ParticleInfoList.IsValidIndex(ParticleID))
					{
						ParticleInfoList.RemoveAt(ParticleID);
					}
				}
			},
			NewParticle.Rate,
			true,
			NewParticle.Delay
		);
	}
}

void AAOSCharacterBase::SpawnLevelUpParticle(int32 OldLevel, int32 NewLevel)
{
	if (OldLevel == NewLevel)
	{
		return;
	}

	if (!::IsValid(LevelUpParticle))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAOSCharacterBase::SpawnLevelUpParticle] LevelUpParticle is invalid."));
		return;
	}

	LastCharacterLocation = GetActorLocation() - FVector(0, 0, 75.f);
	SpawnRootedParicleAtLocation_Server(LevelUpParticle, FTransform(FRotator(1), LastCharacterLocation, FVector(1)));
}


//==================== Mesh Functions ====================//

void AAOSCharacterBase::SpawnAttachedMeshAtLocation_Server_Implementation(UStaticMesh* MeshToSpawn, FVector Location, float Duration)
{
	if (HasAuthority())
	{
		SpawnAttachedMeshAtLocation_Multicast(MeshToSpawn, Location, Duration);
	}
}

void AAOSCharacterBase::SpawnAttachedMeshAtLocation_Multicast_Implementation(UStaticMesh* MeshToSpawn, FVector Location, float Duration)
{
	if (!HasAuthority() && ::IsValid(MeshToSpawn))
	{
		FTimerHandle NewTimerHandle;

		UStaticMeshComponent* NewMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());
		if (::IsValid(NewMeshComponent))
		{
			FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);

			NewMeshComponent->RegisterComponent();
			NewMeshComponent->AttachToComponent(GetRootComponent(), AttachmentRules);
			NewMeshComponent->CreationMethod = EComponentCreationMethod::Instance;
			NewMeshComponent->SetStaticMesh(MeshToSpawn);
			NewMeshComponent->SetCollisionProfileName("CharacterMesh");

			GetWorldTimerManager().SetTimer(
				NewTimerHandle,
				[MeshComponent = NewMeshComponent]()
				{
					if (MeshComponent)
					{
						MeshComponent->DestroyComponent();
					}
				},
				Duration,
				false
			);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create new mesh component."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid MeshToSpawn or unauthorized access."));
	}
}

//==================== Actor Functions ====================//

void AAOSCharacterBase::SpawnActorAtLocation_Server_Implementation(UClass* SpawnActor, FTransform SpawnTransform)
{
	if (HasAuthority())
	{
		if (!SpawnActor)
		{
			return;
		}

		GetWorld()->SpawnActor<AActor>(SpawnActor, SpawnTransform);
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

void AAOSCharacterBase::StopAllMontages_Server_Implementation(float BlendOut)
{
	StopAllMontages_NetMulticast(BlendOut);
}

void AAOSCharacterBase::StopAllMontages_NetMulticast_Implementation(float BlendOut)
{
	if (!HasAuthority() && GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
	{
		if (::IsValid(AnimInstance) == false)
		{
			return;
		}

		AnimInstance->StopAllMontages(BlendOut);
	}
}

void AAOSCharacterBase::PlayMontage_Server_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayMontage_NetMulticast(Montage, PlayRate);
}

void AAOSCharacterBase::PlayMontage_NetMulticast_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (!HasAuthority() && GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
	{
		if (::IsValid(AnimInstance) == false)
		{
			return;
		}

		AnimInstance->PlayMontage(Montage, PlayRate);
	}
}

void AAOSCharacterBase::MontageJumpToSection_Server_Implementation(UAnimMontage* Montage, FName SectionName, float PlayRate)
{
	MontageJumpToSection_NetMulticast(Montage, SectionName, PlayRate);
}

void AAOSCharacterBase::MontageJumpToSection_NetMulticast_Implementation(UAnimMontage* Montage, FName SectionName, float PlayRate)
{
	if (!HasAuthority() && GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
	{
		if (::IsValid(AnimInstance) == false)
		{
			return;
		}

		AnimInstance->Montage_SetPlayRate(Montage, PlayRate);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
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

				float MaxHP = StatComponent->GetMaxHP();
				float CurrentHP = StatComponent->GetCurrentHP();
				float HPRegeneration = StatComponent->GetHealthRegeneration();

				float ClampHP = FMath::Clamp<float>(CurrentHP + HPRegeneration, 0.f, MaxHP);
				StatComponent->SetCurrentHP(ClampHP);
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

				float CurrentMP = StatComponent->GetCurrentMP();
				float MaxMP = StatComponent->GetMaxMP();
				float MPRegeneration = StatComponent->GetManaRegeneration();

				float ClampMP = FMath::Clamp<float>(CurrentMP + MPRegeneration, 0.f, MaxMP);
				StatComponent->SetCurrentMP(ClampMP);
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
	StatComponent->SetCurrentHP(StatComponent->GetCurrentHP() - 100.f);
}

void AAOSCharacterBase::DecreaseMP_Server_Implementation()
{
	StatComponent->SetCurrentMP(StatComponent->GetCurrentMP() - 100.f);
}

void AAOSCharacterBase::IncreaseEXP_Server_Implementation()
{
	StatComponent->SetCurrentEXP(StatComponent->GetCurrentEXP() + 20);
}

void AAOSCharacterBase::IncreaseLevel_Server_Implementation()
{
	StatComponent->SetCurrentLevel(StatComponent->GetCurrentLevel() + 1);
}

void AAOSCharacterBase::IncreaseCriticalChance_Server_Implementation()
{
	StatComponent->SetCriticalChance(StatComponent->GetCriticalChance() + 5);
}

void AAOSCharacterBase::IncreaseAttackSpeed_Server_Implementation()
{
	StatComponent->SetAttackSpeed(StatComponent->GetAttackSpeed() + 0.15f);
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

	if (AAOSGameMode* GM = Cast<AAOSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		if (AAOSPlayerController* AOSPlayerController = Cast<AAOSPlayerController>(GetController()))
		{
			GM->RequestRespawn(AOSPlayerController);
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

	EnumAddFlags(CharacterState, EBaseCharacterState::Dead);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	//SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	ActivatePostProcessEffect_Client();

	if (!::IsValid(AnimInstance) || !::IsValid(Death_Montage))
	{
		SetActorHiddenInGame(true);
		return;
	}

	AnimInstance->PlayMontage(Death_Montage, 1.0f);
}

void AAOSCharacterBase::ActivatePostProcessEffect_Client_Implementation()
{
	/*CameraComponent->PostProcessSettings.ColorSaturation = FVector4(0, 0, 0, 1);
	CameraComponent->PostProcessSettings.ColorGamma = FVector4(1, 1, 1, 0.8f);*/

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
}

void AAOSCharacterBase::Respawn()
{
	UE_LOG(LogTemp, Log, TEXT("[AAOSCharacterBase::Respawn] Calling Respawn Function."));

	EnumRemoveFlags(CharacterState, EBaseCharacterState::Dead);

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

	StatComponent->SetCurrentHP(StatComponent->GetMaxHP());
	StatComponent->SetCurrentMP(StatComponent->GetMaxMP());

	DeActivatePostProcessEffect_Client();
}

float AAOSCharacterBase::SetAnimPlayRate(const float AnimLength)
{
	if (!::IsValid(StatComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("SetAnimPlayRate: StatComponentis null."));
		return 1.0f; // �⺻ ��� �ӵ� ��ȯ
	}

	float CurrentAttackSpeed = StatComponent->GetAttackSpeed();
	float AttackIntervalTime = 1.0f / CurrentAttackSpeed;

	// �ִϸ��̼� ��� �ӵ� ���
	return (AttackIntervalTime < AnimLength) ? (AnimLength / AttackIntervalTime) : 1.0f;
}

//==================== Character Control Functions ====================//

void AAOSCharacterBase::EnableCharacterMove()
{
	EnumAddFlags(CharacterState, EBaseCharacterState::Move);
}

void AAOSCharacterBase::EnableSwitchAtion()
{
	EnumAddFlags(CharacterState, EBaseCharacterState::SwitchAction);
}

void AAOSCharacterBase::EnableUseControllerRotationYaw()
{
	bUseControllerRotationYaw = true;
}

//==================== Utility Functions ====================//

const FName AAOSCharacterBase::GetAttackMontageSection(const int32& Section)
{
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}

