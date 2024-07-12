// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class UUW_ChatWindow;

/**
 *
 */
UCLASS()
class PROJECT_AOS_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALobbyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// UI ���� �Լ�
	void BindChatWindow(UUW_ChatWindow* Widget);
	void ShowLobbyUI();

	UFUNCTION(Client, Reliable)
	void ShowLoadingScreen_Client();

	UFUNCTION(Server, Reliable)
	void ShowChampionSelectUI_Server();

	UFUNCTION(Client, Reliable)
	void ShowChampionSelectUI_Client();

	UFUNCTION(Client, Reliable)
	void SetHostPlayer_Client(bool bIsHost);

	// ä�� ���� �Լ�
	UFUNCTION(Server, Reliable)
	void UpdateChatLog_Server(const FString& Message, AController* EventInstigator);

	UFUNCTION(Client, Reliable)
	void UpdateChatLog_Client(const FString& Text);

	// ĳ���� ���� ���� ������Ʈ �Լ�
	UFUNCTION(Server, Reliable)
	void UpdatePlayerSelection_Server(ETeamSideBase Team, int32 PlayerIndex, const FString& InPlayerName, int32 CharacterIndex, FLinearColor Color, bool bShowCharacterDetails);

	UFUNCTION(Client, Reliable)
	void UpdatePlayerSelection_Client(ETeamSideBase Team, int32 PlayerIndex, const FString& InPlayerName, int32 CharacterIndex, FLinearColor Color, bool bShowCharacterDetails);

	// ���� �ð� ������Ʈ �Լ�
	UFUNCTION(Client, Reliable)
	void UpdateBanPickTime_Client(float InCurrentDraftTime, float InMaxDraftTime);

	// ȣ��Ʈ �÷��̾� ���� Ȯ�� �Լ�
	bool GetIsHostPlayer() const { return bIsHostPlayer; }

protected:
	// ���� �ν��Ͻ�, ���� ���, ���� ����, �÷��̾� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAOSGameInstance> AOSGameInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameMode> LobbyGameMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameState> LobbyGameState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyPlayerState> LobbyPlayerState;

	// UI Ŭ������ �ν��Ͻ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class ULobbyUI> LobbyUIClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ULobbyUI> LobbyUIInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UChampionSelectionUI> ChampionSelectUIClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UChampionSelectionUI>ChampionSelectUIInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> LoadingScreenClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> LoadingScreenInstance;

	// ä�� ������
	UPROPERTY(Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_ChatWindow> ChatWindow;

	// ȣ��Ʈ �÷��̾� ����
	bool bIsHostPlayer;

	// �巡��Ʈ Ÿ�� ���� ����
	bool bChangedDraftTime = false;
	float DraftTime = 0.f;
	float CurrentDraftTime = 0.f;
	float MaxDraftTime = 0.f;
};

