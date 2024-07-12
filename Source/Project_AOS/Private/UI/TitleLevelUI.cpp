// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TitleLevelUI.h"
#include "UI/UW_Button.h"
#include "UI/UW_ConfirmationQuit.h"
#include "Components/Button.h"
#include "Controllers/UIPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Plugins/MultiplaySessionSubsystem.h"

UTitleLevelUI::UTitleLevelUI(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

void UTitleLevelUI::NativeConstruct()
{
	Super::NativeConstruct();

	UMultiplaySessionSubsystem* SessionSystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();

	HostGameButton->SetButtonText("Host Game");
	JoinGameButton->SetButtonText("Join Game");
	QuitButton->SetButtonText("Quit");

	HostGameButton->PlayConstructAnimation();
	JoinGameButton->PlayConstructAnimation();
	QuitButton->PlayConstructAnimation();

	LoginButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::ProcessLogin);
	HostGameButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::CreateHostMenu);
	JoinGameButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::CreateJoinMenu);
	QuitButton->Button->OnClicked.AddDynamic(this, &UTitleLevelUI::CreateQuitMenu);
	SessionSystem->OnLoginCompleteEvent.AddUObject(this, &UTitleLevelUI::OnLoginCompleted);
}

void UTitleLevelUI::ProcessLogin()
{
	UMultiplaySessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
	if (::IsValid(SessionSubsystem))
	{
		SessionSubsystem->LoginWithEOS("", "", FString("AccountPortal"));
	}
}

void UTitleLevelUI::CreateHostMenu()
{
	AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (::IsValid(PlayerController))
	{
		PlayerController->ShowHostMenu();
	}
}

void UTitleLevelUI::CreateJoinMenu()
{
	AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (::IsValid(PlayerController))	
	{
		PlayerController->ShowJoinMenu();
	}

	UMultiplaySessionSubsystem* SessionSystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
	if (::IsValid(SessionSystem))
	{
		SessionSystem->FindSessions(10, false);
	}
}

void UTitleLevelUI::CreateQuitMenu()
{
	AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (::IsValid(PlayerController))
	{
		PlayerController->ShowConfirmationQuitMenu();
	}
}

void UTitleLevelUI::OnLoginCompleted(bool Successful)
{
	if (Successful)
	{
		HostGameButton->SetIsEnabled(true);
		JoinGameButton->SetIsEnabled(true);
	}
}
