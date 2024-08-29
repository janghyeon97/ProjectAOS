// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/SlowEffect.h"
#include "Characters/CharacterBase.h"
#include "Components/StatComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "GameFramework/CharacterMovementComponent.h"

void USlowEffect::ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent)
{
    if (!InTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is null."));
        return;
    }

    UWorld* World = InTarget->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyEffect failed: Target's GetWorld() returned null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(InTarget);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    UStatComponent* StatComponent = Character->GetStatComponent();
    if (!StatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: StatComponent is null for the target."));
        return;
    }

    // ���� ����� ���� ������ ��ȭ ȿ���� ���
    float MaxSlowPercent = Percent;
    Target = InTarget;

    // ���ο� ȿ���� �� ������ ���, ���̸� ����Ͽ� �̵� �ӵ��� �ݿ�
    if (FMath::Abs(InPercent) > FMath::Abs(MaxSlowPercent))
    {
        float Difference = InPercent - MaxSlowPercent;
        Percent = InPercent;

        // �̵� �ӵ��� ���̸� �ݿ�
        StatComponent->ModifyAccumulatedPercentMovementSpeed(-Difference);
        EnumAddFlags(Character->CrowdControlState, EBaseCrowdControl::Slow);

        World->GetTimerManager().ClearTimer(TimerHandle);
        World->GetTimerManager().SetTimer(
            TimerHandle,
            [this]()
            {
                this->RemoveEffect();
            },
            InDuration,
            false
        );
    }
    else
    {
        // ���ο� ȿ���� �� ���� ���, �迭���� �߰��ϰ� ���߿� ó��
        FActiveSlowEffect NewEffect(InPercent, InDuration, GetWorld()->GetTimeSeconds());
        ActiveSlowEffects.Add(NewEffect);
    }
}

void USlowEffect::RemoveEffect()
{
    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is null."));
        return;
    }

    UWorld* World = Target->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyEffect failed: Target's GetWorld() returned null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(Target);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    UStatComponent* StatComponent = Character->GetStatComponent();
    if (!StatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: StatComponent is null for the target."));
        return;
    }

    // ����� ȿ���� �迭���� �����ϰ�, ���� ������ ȿ���� ã��
    float CurrentTime = GetWorld()->GetTimeSeconds();
    ActiveSlowEffects.RemoveAll([CurrentTime](const FActiveSlowEffect& Effect)
        {
            return (CurrentTime - Effect.StartTime) >= Effect.Duration;
        });

    // �����ִ� ��ȭ ȿ�� �� ���� ������ ȿ���� ���
    float MaxSlowPercent = 0.f;
    for (const FActiveSlowEffect& Effect : ActiveSlowEffects)
    {
        if (FMath::Abs(Effect.SlowPercent) > FMath::Abs(MaxSlowPercent))
        {
            MaxSlowPercent = Effect.SlowPercent;
        }
    }

    // ���� ȿ���� ���Ͽ� ���̸�ŭ �̵� �ӵ� ����
    float Difference = MaxSlowPercent - Percent;
    StatComponent->ModifyAccumulatedPercentMovementSpeed(-Difference);

    // ���ο� ȿ���� Percent �� ����
    Percent = MaxSlowPercent;

    // ����� ȿ���� ������ Ÿ�̸Ӹ� ���� ����
    if (MaxSlowPercent != 0.f)
    {
        float NextExpiration = FLT_MAX;
        for (const FActiveSlowEffect& Effect : ActiveSlowEffects)
        {
            float ExpirationTime = Effect.StartTime + Effect.Duration - CurrentTime;
            if (ExpirationTime < NextExpiration)
            {
                NextExpiration = ExpirationTime;
            }
        }

        World->GetTimerManager().SetTimer(
            TimerHandle,
            [this]()
            {
                this->RemoveEffect();
            },
            NextExpiration,
            false
        );
    }
    else
    {
        // ��� ȿ���� ����Ǹ� Percent �ʱ�ȭ
        Percent = 0.f;
        EnumRemoveFlags(Character->CrowdControlState, EBaseCrowdControl::Slow);
        ReturnEffect();
    }
}

void USlowEffect::ReturnEffect()
{
    if (!::IsValid(Target))
    {
        UE_LOG(LogTemp, Warning, TEXT("ReturnEffect failed: Target is null."));
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(Target);
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
        return;
    }

    // ActiveEffects���� ȿ�� ����
    if (Character->ActiveEffects.Remove(EBaseCrowdControl::Slow) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Removed Stun effect from ActiveEffects."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect: ActiveEffects does not contain Stun effect."));
    }

    // ȿ�� ��ü�� Ǯ�� ��ȯ
    if (UCrowdControlManager* CrowdControlManager = UCrowdControlManager::Get())
    {
        if (::IsValid(CrowdControlManager))
        {
            CrowdControlManager->ReturnEffect(EBaseCrowdControl::Slow, this);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Retrieved CrowdControlManager instance is invalid."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Could not retrieve UCrowdControlManager instance."));
    }
}

void USlowEffect::Reset()
{
    ActiveSlowEffects.Empty();
    Duration = 0.0f;
    Percent = 0.0f;
}

float USlowEffect::GetDuration() const
{
    return Duration;
}

float USlowEffect::GetPercent() const
{
    return Percent;
}