// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LoadingScreenUI.h"
#include "Plugins/MultiplaySessionSubsystem.h"

void ULoadingScreenUI::NativeConstruct()
{
	UMultiplaySessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
	SessionSubsystem->OnCreateSessionCompleteEvent.AddDynamic(this, &ULoadingScreenUI::RemoveLoadingScreen);
}

void ULoadingScreenUI::RemoveLoadingScreen(bool Successful)
{
	if(Successful) RemoveFromParent();
}
