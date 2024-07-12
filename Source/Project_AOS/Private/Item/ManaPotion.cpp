// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ManaPotion.h"
#include "Game/AOSPlayerState.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"

AManaPotion::AManaPotion()
{
	ManaRegenerationIncrement = 10.f;
    ManaRegenDuration = 15.f;
    ManaRegenInterval = 0.5f;
}

void AManaPotion::Use(AAOSPlayerState* PlayerState)
{
    if (!::IsValid(PlayerState))
    {
        return;
    }

    APlayerController* PlayerController = PlayerState->GetPlayerController();
    if (!::IsValid(PlayerController))
    {
        return;
    }

    AAOSCharacterBase* PlayerCharacter = PlayerController->GetPawn<AAOSCharacterBase>();
    if (!::IsValid(PlayerCharacter))
    {
        return;
    }

    UStatComponent* PlayerStatComponent = PlayerCharacter->GetStatComponent();
    if (!::IsValid(PlayerStatComponent))
    {
        return;
    }

    // 아이템 식별자를 사용하여 타이머를 관리
    auto TimerCallback = [WeakPlayerStatComponent = TWeakObjectPtr<UStatComponent>(PlayerStatComponent), ManaRegeneration = ManaRegenerationIncrement]()
        {
            if (UStatComponent* ValidPlayerStatComponent = WeakPlayerStatComponent.Get())
            {
                ValidPlayerStatComponent->SetCurrentMP(ValidPlayerStatComponent->GetCurrentMP() - ManaRegeneration);
            }
        };

    int32 MultipliedItemID = ItemID * 1024;

    // 타이머 종료 콜백 함수
    auto EndTimerCallback = [PlayerState, MultipliedItemID]()
        {
            PlayerState->ClearItemTimer(MultipliedItemID);
        };

    // 힐링 포션 타이머가 이미 작동 중인지 확인
    if (PlayerState->IsItemTimerActive(MultipliedItemID))
    {
        float RemainingTime = PlayerState->GetItemTimerRemaining(MultipliedItemID);
        float NewDuration = RemainingTime + ManaRegenDuration;

        PlayerState->ClearItemTimer(MultipliedItemID);
        PlayerState->SetItemTimer(MultipliedItemID, EndTimerCallback, NewDuration, false);
    }
    else
    {
        PlayerState->SetItemTimer(ItemID, TimerCallback, ManaRegenInterval, true, ManaRegenInterval);
        PlayerState->SetItemTimer(MultipliedItemID, EndTimerCallback, ManaRegenDuration, false);
    }

    CurrentStack--;

    // 아이템 스택이 0 이하이면 인벤토리에서 제거
    if (CurrentStack <= 0)
    {
        if (PlayerState->HasAuthority())
        {
            PlayerState->RemoveItemFromInventory(ItemID);
        }
    }

    Super::Use(PlayerState);
}