// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/AOSPlayerController.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Game/AOSPlayerState.h"
#include "Game/AOSGameMode.h"
#include "Game/AOSGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LoadingScreenUI.h"
#include "UI/UW_ItemShop.h"
#include "UI/UHUD.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AAOSPlayerController::AAOSPlayerController()
{

}

void AAOSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		ShowLoadingScreen();

		ItemShopWidget = CreateWidget<UUW_ItemShop>(this, ItemShopWidgetClass);
		if (::IsValid(ItemShopWidget))
		{
			ItemShopWidget->AddToViewport(1);
			ItemShopWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
	if (!HasAuthority() && IsLocalPlayerController())
	{
		float RandomTime = FMath::RandRange(1.0f, 3.0f);
		FTimerHandle NewTimer;
		GetWorld()->GetTimerManager().SetTimer(NewTimer, FTimerDelegate::CreateUObject(this, &AAOSPlayerController::ServerNotifyLoaded), RandomTime, false);
	}
}

void AAOSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SelectedCharacterIndex);
}

void AAOSPlayerController::InitializeHUD_Implementation(const int InChampionIndex, const FName& InChampionName)
{
	FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);

	if (::IsValid(HUDWidgetClass))
	{
		HUDWidget = CreateWidget<UUHUD>(this, HUDWidgetClass);
		if (::IsValid(HUDWidget))
		{
			HUDWidget->OnComponentsBindingCompleted.AddUObject(this, &ThisClass::OnHUDBindingComplete);

			AAOSCharacterBase* PlayerCharacter = GetPawn<AAOSCharacterBase>();
			if (::IsValid(PlayerCharacter))
			{
				UStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
				UAbilityStatComponent* AbilityStatComponent = PlayerCharacter->GetAbilityStatComponent();
				AAOSPlayerState* AOSPlayerState = Cast<AAOSPlayerState>(PlayerState);
				AAOSGameState* AOSGameState = Cast<AAOSGameState>(UGameplayStatics::GetGameState(GetWorld()));

				if (::IsValid(StatComponent) && ::IsValid(AbilityStatComponent) && ::IsValid(AOSPlayerState) && ::IsValid(AOSGameState))
				{
					HUDWidget->BindComponents(AOSGameState, AOSPlayerState, StatComponent, AbilityStatComponent);
					HUDWidget->InitializeHUD(InChampionIndex, InChampionName);
					HUDWidget->SetOwningActor(PlayerCharacter);
				}
			}
			HUDWidget->AddToViewport();
		}
	}
}

void AAOSPlayerController::ShowLoadingScreen_Implementation()
{
	if (::IsValid(LoadingScreenInstance))
	{
		LoadingScreenInstance->AddToViewport(1);
	}
	else
	{	
		LoadingScreenInstance = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		if (::IsValid(LoadingScreenInstance))
		{
			LoadingScreenInstance->AddToViewport(1);
		}
	}
}

void AAOSPlayerController::RemoveLoadingScreen_Implementation()
{
	if (::IsValid(LoadingScreenInstance))
	{
		LoadingScreenInstance->RemoveFromParent();
	}
}

void AAOSPlayerController::ServerNotifyLoaded_Implementation()
{
	// 서버에 로딩 완료 알림
	if (AAOSGameMode* GM = Cast<AAOSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->PlayerLoaded(this);
	}
}

void AAOSPlayerController::ToggleItemShopVisibility_Implementation()
{
	if (ItemShopWidget)
	{
		if (bItemShopVisibility)
		{
			// 닫기
			ItemShopWidget->SetVisibility(ESlateVisibility::Hidden);

			FInputModeGameOnly Mode;
			SetInputMode(Mode);

			bShowMouseCursor = false;
			bEnableClickEvents = false;
			bEnableMouseOverEvents = false;

			UE_LOG(LogTemp, Warning, TEXT("ItemShop Closed"));
		}
		else
		{
			// 열기
			ItemShopWidget->SetVisibility(ESlateVisibility::Visible);

			FInputModeGameAndUI Mode;
			Mode.SetWidgetToFocus(ItemShopWidget->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(Mode);

			bShowMouseCursor = true;
			bEnableClickEvents = true;
			bEnableMouseOverEvents = true;

			UE_LOG(LogTemp, Warning, TEXT("ItemShop Opened"));
		}

		bItemShopVisibility = !bItemShopVisibility;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ItemShopWidget is not valid"));
	}
}

void AAOSPlayerController::InitializeItemShop_Implementation()
{
	if (!::IsValid(ItemShopWidget))
	{
		return;
	}

	if (AAOSCharacterBase* PlayerCharacter = GetPawn<AAOSCharacterBase>())
	{
		if (AAOSPlayerState* AOSPlayerState = Cast<AAOSPlayerState>(PlayerState))
		{
			ItemShopWidget->BindPlayerState(AOSPlayerState);
			ItemShopWidget->SetOwningActor(PlayerCharacter);
		}
	}
}

void AAOSPlayerController::OnRep_PlayerInfoReplicated()
{

}

void AAOSPlayerController::OnHUDBindingComplete_Implementation()
{
	int32 InitialCharacterLevel = 1;

	if (AAOSGameMode* GM = Cast<AAOSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		InitialCharacterLevel = GM->GetInitialCharacterLevel();
	}

	AAOSCharacterBase* PlayerCharacter = GetPawn<AAOSCharacterBase>();
	if (!::IsValid(PlayerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAOSPlayerController::OnHUDBindingComplete] PlayerCharacter is not valid"));
		return;
	}

	UStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
	UAbilityStatComponent* AbilityStatComponent = PlayerCharacter->GetAbilityStatComponent();

	if (!::IsValid(StatComponent) || !::IsValid(AbilityStatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AAOSPlayerController::OnHUDBindingComplete] StatComponent or AbilityStatComponent is not valid"));
		return;
	}

	AbilityStatComponent->InitializeAbility(EAbilityID::Ability_LMB, 1);
	StatComponent->SetCurrentLevel(InitialCharacterLevel);
	
	if (OnInitializationCompleted.IsBound())
	{
		OnInitializationCompleted.Broadcast();
	}

	RemoveLoadingScreen();

	UE_LOG(LogTemp, Warning, TEXT("[AAOSPlayerController::OnHUDBindingComplete] InitialCharacterLevel set to %d"), InitialCharacterLevel);
}


void AAOSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

}

void AAOSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Log, TEXT("Possessed pawn: %s"), *InPawn->GetName());

	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(InPawn);
	if (OwningCharacter)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully possessed AAOSCharacterBase: %s"), *OwningCharacter->GetName());

		// PlayerState의 소유자가 올바르게 설정되었는지 확인합니다.
		if (PlayerState)
		{
			PlayerState->SetOwner(this); // PlayerState의 소유자를 PlayerController로 설정
			UE_LOG(LogTemp, Log, TEXT("PlayerState's owner set to: %s"), *GetNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to possess AAOSCharacterBase"));
	}
}
