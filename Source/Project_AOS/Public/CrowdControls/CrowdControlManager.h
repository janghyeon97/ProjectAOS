// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/CustomCombatData.h"
#include "CrowdControlManager.generated.h"

class UCrowdControlEffectBase;

USTRUCT(BlueprintType)
struct FCrowdControlEffectPool
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<UCrowdControlEffectBase*> EffectPool;
};

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UCrowdControlManager : public UObject
{
	GENERATED_BODY()

public:
    UCrowdControlManager();

    // �� ���� ���� ȿ���� ���� ��ü�� �̸� ����
    void InitializeEffectPools();

    // ȿ�� Ŭ���� ��������
    TSubclassOf<UCrowdControlEffectBase> GetEffectClass(EBaseCrowdControl Type) const;

    // ��ü Ǯ���� ��� ������ ȿ�� ��������
    UCrowdControlEffectBase* GetEffect(EBaseCrowdControl Type);

    // ����� ���� ȿ���� Ǯ�� ��ȯ
    void ReturnEffect(EBaseCrowdControl Type, UCrowdControlEffectBase* Effect);

    static UCrowdControlManager* Get()
    {
        if (!Instance)
        {
            Instance = NewObject<UCrowdControlManager>();
            Instance->AddToRoot(); // Garbage Collection ����
            Instance->InitializeEffectPools();
        }
        return Instance;
    }

    static void Release()
    {
        if (Instance)
        {
            Instance->RemoveFromRoot(); // Garbage Collection�� �ٽ� �߰�
            Instance = nullptr; // �����͸� null�� �����Ͽ� �����ϰ� ����
        }
    }

private:
    static UCrowdControlManager* Instance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CrowdControlsMap", meta = (AllowPrivateAccess = "true"))
    TMap<EBaseCrowdControl, TSubclassOf<UCrowdControlEffectBase>> CrowdControlClasses;

    // ��ü Ǯ ����
    UPROPERTY()
    TMap<EBaseCrowdControl, FCrowdControlEffectPool> EffectPools;

    // �ʱ� Ǯ ũ�� ����
    const int32 PoolSizePerType = 10;
};
