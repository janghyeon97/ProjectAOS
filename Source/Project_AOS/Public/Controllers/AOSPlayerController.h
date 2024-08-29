// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AOSPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitializationCompletedDelegate);

class UUHUD;

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AAOSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAOSPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	UUHUD* GetHUDWidget() const { return HUDWidget; }

	UFUNCTION(Client, Reliable)
	void ShowLoadingScreen();

	UFUNCTION(Client, Reliable)
	void RemoveLoadingScreen();

	UFUNCTION(Client, Reliable)
	void InitializeHUD(const int InChampionIndex, const FName& InChampionName);

	UFUNCTION(Client, Reliable)
	void InitializeItemShop();

	UFUNCTION(Server, Reliable)
	void ServerNotifyLoaded();

	UFUNCTION(Client, Reliable)
	void ToggleItemShopVisibility();

	UFUNCTION()
	void OnRep_PlayerInfoReplicated();
	
	UFUNCTION(Server, Reliable)
	void OnHUDBindingComplete();

public:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerInfoReplicated)
	uint32 SelectedCharacterIndex;

	FOnInitializationCompletedDelegate OnInitializationCompleted;

private:
	UPROPERTY()
	TObjectPtr<class UUHUD> HUDWidget;

	UPROPERTY()
	TObjectPtr<class UUW_ItemShop> ItemShopWidget;

	UPROPERTY()
	TObjectPtr<class UUW_StateBar> StateBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AOSPlayerController", Meta = (AllowPrivateAccess));
	TSubclassOf<class UUHUD> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AOSPlayerController", Meta = (AllowPrivateAccess));
	TSubclassOf<class UUW_ItemShop> ItemShopWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AOSPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> LoadingScreenClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AOSPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> LoadingScreenInstance;

	bool bItemShopVisibility = false;
};
