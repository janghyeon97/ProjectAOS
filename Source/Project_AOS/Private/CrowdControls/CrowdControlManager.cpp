// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/CrowdControlManager.h"
#include "CrowdControls/StunEffect.h"
#include "CrowdControls/SlowEffect.h"
#include "CrowdControls/SnareEffect.h"


UCrowdControlManager* UCrowdControlManager::Instance = nullptr;

UCrowdControlManager::UCrowdControlManager()
{
	static ConstructorHelpers::FClassFinder<UStunEffect> StunEffectClass(TEXT("/Script/Project_AOS.StunEffect"));
	if (StunEffectClass.Succeeded()) CrowdControlClasses.Add(EBaseCrowdControl::Stun, StunEffectClass.Class);

	static ConstructorHelpers::FClassFinder<USlowEffect> SlowEffectClass(TEXT("/Script/Project_AOS.SlowEffect"));
	if (SlowEffectClass.Succeeded()) CrowdControlClasses.Add(EBaseCrowdControl::Slow, SlowEffectClass.Class);

	static ConstructorHelpers::FClassFinder<USnareEffect> SnareEffectClass(TEXT("/Script/Project_AOS.SnareEffect"));
	if (SnareEffectClass.Succeeded()) CrowdControlClasses.Add(EBaseCrowdControl::Snare, SnareEffectClass.Class);
}



void UCrowdControlManager::InitializeEffectPools()
{
	for (const auto& Elem : CrowdControlClasses)
	{
		EBaseCrowdControl Type = Elem.Key;
		TSubclassOf<UCrowdControlEffectBase> EffectClass = Elem.Value;

		FCrowdControlEffectPool PoolStruct;

		for (int32 i = 0; i < PoolSizePerType; ++i)
		{
			UCrowdControlEffectBase* NewEffect = NewObject<UCrowdControlEffectBase>(this, EffectClass);
			PoolStruct.EffectPool.Add(NewEffect);
		}

		EffectPools.Add(Type, PoolStruct);
	}
}



TSubclassOf<UCrowdControlEffectBase> UCrowdControlManager::GetEffectClass(EBaseCrowdControl Type) const
{
	if (const TSubclassOf<UCrowdControlEffectBase>* FoundClass = CrowdControlClasses.Find(Type))
	{
		if (::IsValid(*FoundClass))
		{
			return *FoundClass;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid class found for crowd control type: %d"), static_cast<int32>(Type));
		}
	}

	return nullptr;
}

UCrowdControlEffectBase* UCrowdControlManager::GetEffect(EBaseCrowdControl Type)
{
	if (EffectPools.Contains(Type))
	{
		TArray<UCrowdControlEffectBase*>& Pool = EffectPools[Type].EffectPool;
		if (Pool.Num() > 0)
		{
			UCrowdControlEffectBase* Effect = Pool.Pop();
			return Effect;
		}
		else
		{
			// 풀이 비어 있으면 새로운 객체 생성
			TSubclassOf<UCrowdControlEffectBase> EffectClass = GetEffectClass(Type);
			if (EffectClass)
			{
				return NewObject<UCrowdControlEffectBase>(this, EffectClass);
			}
		}
	}

	return nullptr;
}

void UCrowdControlManager::ReturnEffect(EBaseCrowdControl Type, UCrowdControlEffectBase* Effect)
{
	if (Effect && EffectPools.Contains(Type))
	{
		EffectPools[Type].EffectPool.Add(Effect);
		Effect->Reset();
	}
}