// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Structs/EnumTeamSide.h"
#include "LobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()


public:
	ALobbyPlayerState();

	FName GetPlayerUniqueIdString() const;

	void SetSelectedChampionIndex(int32 Index) { SelectedChampionIndex = Index; };

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void UpdateSelectedChampion_Server(const int32 Index, const FName& InName);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAOSPlayerState", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameState> LobbyGameState;

public:
	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	ETeamSideBase TeamSide;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	FName PlayerUniqueID;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	int32 PlayerIndex;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	int32 SelectedChampionIndex;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	FName SelectedChampionName;
};
