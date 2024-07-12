// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ConfirmationQuit.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/UW_Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


UUW_ConfirmationQuit::UUW_ConfirmationQuit(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UUW_ConfirmationQuit::NativeConstruct()
{
	Super::NativeConstruct();

	QuitButton->Button->OnClicked.AddDynamic(this, &UUW_ConfirmationQuit::OnClickQuitButton);
	CancleButton->Button->OnClicked.AddDynamic(this, &UUW_ConfirmationQuit::OnExecuteCancle);

	QuitButton->SetButtonText("Quit");
	CancleButton->SetButtonText("Cancle");
}

void UUW_ConfirmationQuit::OnExecuteQuit_Client_Implementation()
{
	if (GetOwningPlayer() == UGameplayStatics::GetPlayerController(this, 0))
	{
		TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;
		UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(this, 0), QuitPreference, true);
	}
}

void UUW_ConfirmationQuit::OnClickQuitButton()
{
	OnExecuteQuit_Client();
}

void UUW_ConfirmationQuit::OnExecuteCancle()
{
	RemoveFromParent();
}