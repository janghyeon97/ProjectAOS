// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

class ALobbyPlayerController;

DECLARE_MULTICAST_DELEGATE(FOnConnectedPlayerReplicatedDelegate);

USTRUCT(BlueprintType)
struct FPlayerInfomaion : public FTableRowBase
{
	GENERATED_BODY()

public:
	FPlayerInfomaion()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ProfileImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AAOSPlayerState* PlayerState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ALobbyPlayerController* PlayerController;
};

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameState();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	uint8 GetPlayerNum() const { return NumberOfPlayers; };

	UFUNCTION()
	void UpdatePlayers();    
	 
	UFUNCTION()
	void OnRep_ConnectedPlayerChanged();

	FOnConnectedPlayerReplicatedDelegate OnConnectedPlayerReplicated;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameMode> LobbyGameMode;

public:
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayerChanged, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TArray<TObjectPtr<class ALobbyPlayerState>> BlueTeamPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayerChanged, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TArray<TObjectPtr<class ALobbyPlayerState>> RedTeamPlayers;

	uint8 ProxyBanPickPhase = 0;

	uint8 NumberOfPlayers = 0;
};