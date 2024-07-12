// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/EnumClassification.h"
#include "Structs/StructItemAbility.h"
#include "Item.generated.h"

class AAOSCharacterBase;
class UStatComponent;
class AAOSPlayerState;

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API AItem : public AActor
{
	GENERATED_BODY()

public:
    AItem();
   
public:
    virtual void Use(AAOSPlayerState* PlayerState);
    virtual void BindToPlayer(AAOSCharacterBase* Character);
    virtual void ApplyAbilitiesToCharacter(AAOSCharacterBase* Character);
    virtual void RemoveAbilitiesFromCharacter();

protected:
    UFUNCTION()
    virtual void OnHit(FDamageInfomation& DamageInfomation);

    UFUNCTION()
    virtual void OnAttack(FDamageInfomation& DamageInfomation);

    UFUNCTION()
    virtual void OnAbilityEffects(FDamageInfomation& DamageInfomation);

    UFUNCTION()
    virtual void OnReceiveDamageEntered(bool& bResult);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Price;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    UTexture* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 MaxStack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 CurrentStack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 MaxPossessQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EClassification Classification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TArray<FItemAbility> Abilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TArray<int> RequiredItems;

protected:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item")
    TWeakObjectPtr<AAOSCharacterBase> OwnerCharacter;

protected:
    using AbilityFunction = void (AItem::*)(AAOSCharacterBase*, int32);

    void ModifyMaxHealthPoints(AAOSCharacterBase* Character, int32 Value);
    void ModifyMaxManaPoints(AAOSCharacterBase* Character, int32 Value);
    void ModifyHealthRegeneration(AAOSCharacterBase* Character, int32 Value);
    void ModifyManaRegeneration(AAOSCharacterBase* Character, int32 Value);
    void ModifyAttackDamage(AAOSCharacterBase* Character, int32 Value);
    void ModifyAbilityPower(AAOSCharacterBase* Character, int32 Value);
    void ModifyDefensePower(AAOSCharacterBase* Character, int32 Value);
    void ModifyMagicResistance(AAOSCharacterBase* Character, int32 Value);
    void ModifyAttackSpeed(AAOSCharacterBase* Character, int32 Value);
    void ModifyAbilityHaste(AAOSCharacterBase* Character, int32 Value);
    void ModifyCriticalChance(AAOSCharacterBase* Character, int32 Value);
    void ModifyMovementSpeed(AAOSCharacterBase* Character, int32 Value);

private:
    TMap<EItemAbility, AbilityFunction> AbilityFunctionMap;

    template <typename T>
    void ModifyStat(AAOSCharacterBase* Character, T(UStatComponent::* Getter)() const, void (UStatComponent::* Setter)(T), int32 Value);
};