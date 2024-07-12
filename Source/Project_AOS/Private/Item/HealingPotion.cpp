#include "Item/HealingPotion.h"
#include "Game/AOSPlayerState.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "TimerManager.h"

AHealingPotion::AHealingPotion()
{
	HealthRegenerationIncrement = 4;
    HealingDuration = 15.f;
    HealingInterval = 0.5f;
}

void AHealingPotion::Use(AAOSPlayerState* PlayerState)
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

    // ������ �ĺ��ڸ� ����Ͽ� Ÿ�̸Ӹ� ����
    auto TimerCallback = [WeakPlayerStatComponent = TWeakObjectPtr<UStatComponent>(PlayerStatComponent), HealthRegeneration = HealthRegenerationIncrement]()
        {
            if (UStatComponent* ValidPlayerStatComponent = WeakPlayerStatComponent.Get())
            {
                ValidPlayerStatComponent->SetCurrentHP(ValidPlayerStatComponent->GetCurrentHP() + HealthRegeneration);
            }
        };

    int32 MultipliedItemID = ItemID * 1024;

    // Ÿ�̸� ���� �ݹ� �Լ�
    auto EndTimerCallback = [PlayerState, MultipliedItemID]()
        {
            PlayerState->ClearItemTimer(MultipliedItemID);
        };

    // ���� ���� Ÿ�̸Ӱ� �̹� �۵� ������ Ȯ��
    if (PlayerState->IsItemTimerActive(MultipliedItemID))
    {
        float RemainingTime = PlayerState->GetItemTimerRemaining(MultipliedItemID);
        float NewDuration = RemainingTime + HealingDuration;

        PlayerState->ClearItemTimer(MultipliedItemID);
        PlayerState->SetItemTimer(MultipliedItemID, EndTimerCallback, NewDuration, false);
    }
    else
    {
        PlayerState->SetItemTimer(ItemID, TimerCallback, HealingInterval, true, HealingInterval);
        PlayerState->SetItemTimer(MultipliedItemID, EndTimerCallback, HealingDuration, false);
    }

    CurrentStack--;

    // ������ ������ 0 �����̸� �κ��丮���� ����
    if (CurrentStack <= 0)
    {
        if (PlayerState->HasAuthority())
        {
            PlayerState->RemoveItemFromInventory(ItemID);
        }
    }

    Super::Use(PlayerState);
}
