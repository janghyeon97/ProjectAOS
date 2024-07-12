// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HostMenuUI.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "UI/UW_EditableText.h"
#include "UI/UW_Button.h"
#include "Plugins/MultiplaySessionSubsystem.h"
#include "Controllers/UIPlayerController.h"
#include "Kismet/GameplayStatics.h"

UHostMenuUI::UHostMenuUI(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

void UHostMenuUI::NativeConstruct()
{
	ConfirmButton->Button->OnClicked.AddDynamic(this, &UHostMenuUI::OnConfirmButtonClicked);
	CancleButton->Button->OnClicked.AddDynamic(this, &UHostMenuUI::RemoveHostMenu);
}

void UHostMenuUI::OnConfirmButtonClicked()
{
	AUIPlayerController* PlayerController = Cast<AUIPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

	if (::IsValid(PlayerController))
	{
		PlayerController->ShowLoadingScreen();
	}

	FTimerHandle NewTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		NewTimerHandle,
		FTimerDelegate::CreateLambda([this]()
			{
				UMultiplaySessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
				SessionSubsystem->CreateSession(true, false, EditableNameText->GetText(), 2, "Rift", EditablePasswordText->GetText());
			}),
		1.0f,
		false,
		2.0f
	);
	
}

void UHostMenuUI::RemoveHostMenu()
{
	RemoveFromParent();
}