#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/EnumTeamSide.h"
#include "ChampionSelectionUI.generated.h"

class UUW_ChampionSelection;

UCLASS()
class PROJECT_AOS_API UChampionSelectionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	class UWrapBox* GetChampionListBox() { return ChampionListBox; };

	void InitializePlayerList();
	void InitializeChampionList();
	void AddChampionListEntry(int32 Index, const FString& InChampionName, UTexture* Texture);

	void AddPlayerSelection(ETeamSideBase Team, const FString& InPlayerName);
	void AddPlayerSelectionUI(TArray<TObjectPtr<UUW_ChampionSelection>>& TeamPlayers, TArray<TObjectPtr<UWidget>>& TeamWidgets, uint8& CurrentIndex, const FString& InPlayerName);

	void UpdatePlayerSelection(ETeamSideBase Team, const int32 PlayerIndex, const FString& InPlayerName, UTexture* Texture, const FString& InChampionName, const FString& InChampionPosition, FLinearColor Color, bool bShowChampionDetails);
	void UpdatePlayerSelectionUI(TArray<TObjectPtr<UUW_ChampionSelection>>& TeamPlayers, const int32 PlayerIndex, const FString& InPlayerName, UTexture* Texture, const FString& InChampionName, const FString& InChampionPosition, FLinearColor Color, bool bShowChampionDetails);

	void UpdateInfomationText(const FString& String);
	void OnBanPickTimeChanged(float CurrentTime, float MaxTime);

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TWeakObjectPtr<class ALobbyGameState> LobbyGameState;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TWeakObjectPtr<class UAOSGameInstance> AOSGameInstance;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UChampionSectionUI", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> ChampionListEntryClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UChampionSectionUI", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> ChampionSelectionClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> InfomationText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> TimerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UProgressBar> BlueProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UProgressBar> RedProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UWrapBox> RedTeamSelection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UWrapBox> BlueTeamSelection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UWrapBox> ChampionListBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UUW_ChatWindow> ChatWindow;

	TArray<TObjectPtr<UUW_ChampionSelection>> BlueTeamPlayers;
	TArray<TObjectPtr<UUW_ChampionSelection>> RedTeamPlayers;
	TArray<TObjectPtr<class UWidget>> BlueTeamWidgets;
	TArray<TObjectPtr<class UWidget>> RedTeamWidgets;

	uint8 BlueTeamCurrentIndex = 0;
	uint8 RedTeamCurrentIndex = 0;
};
