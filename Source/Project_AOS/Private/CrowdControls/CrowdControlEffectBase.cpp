// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/CrowdControlEffectBase.h"


void UCrowdControlEffectBase::ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent)
{
	UE_LOG(LogTemp, Error, TEXT("ApplyEffect is not implemented in %s"), *GetName());
	return;
}

void UCrowdControlEffectBase::RemoveEffect()
{
	UE_LOG(LogTemp, Error, TEXT("RemoveEffect is not implemented in %s"), *GetName());
	return;
}

void UCrowdControlEffectBase::Reset()
{
	UE_LOG(LogTemp, Error, TEXT("Reset is not implemented in %s"), *GetName());
	return;
}

float UCrowdControlEffectBase::GetDuration() const
{
	UE_LOG(LogTemp, Error, TEXT("GetDuration is not implemented in %s"), *GetName());
	return 0.0f;
}

float UCrowdControlEffectBase::GetPercent() const
{
	UE_LOG(LogTemp, Error, TEXT("GetPercent is not implemented in %s"), *GetName());
	return 0.0f;
}

void UCrowdControlEffectBase::ReturnEffect()
{
	UE_LOG(LogTemp, Error, TEXT("ReturnEffect is not implemented in %s"), *GetName());
	return;
}
