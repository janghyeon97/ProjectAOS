#include "Item/Item.h"
#include "Game/AOSPlayerState.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"

AItem::AItem()
    : ItemID(0)
    , Name(TEXT("Default Name"))
    , Price(0)
    , Icon(nullptr)
    , Description(TEXT("Default Description"))
    , MaxStack(1)
    , CurrentStack(0)
    , MaxPossessQuantity(1)
    , Classification(EClassification::None)
    , RequiredItems()
{
    // Ability function mapping for applying and removing abilities
    AbilityFunctionMap.Add(EItemAbility::MaxHealthPoints, &AItem::ModifyMaxHealthPoints);
    AbilityFunctionMap.Add(EItemAbility::MaxManaPoints, &AItem::ModifyMaxManaPoints);
    AbilityFunctionMap.Add(EItemAbility::HealthRegeneration, &AItem::ModifyHealthRegeneration);
    AbilityFunctionMap.Add(EItemAbility::ManaRegeneration, &AItem::ModifyManaRegeneration);
    AbilityFunctionMap.Add(EItemAbility::AttackDamage, &AItem::ModifyAttackDamage);
    AbilityFunctionMap.Add(EItemAbility::AbilityPower, &AItem::ModifyAbilityPower);
    AbilityFunctionMap.Add(EItemAbility::DefensePower, &AItem::ModifyDefensePower);
    AbilityFunctionMap.Add(EItemAbility::MagicResistance, &AItem::ModifyMagicResistance);
    AbilityFunctionMap.Add(EItemAbility::AttackSpeed, &AItem::ModifyAttackSpeed);
    AbilityFunctionMap.Add(EItemAbility::AbilityHaste, &AItem::ModifyAbilityHaste);
    AbilityFunctionMap.Add(EItemAbility::CriticalChance, &AItem::ModifyCriticalChance);
    AbilityFunctionMap.Add(EItemAbility::MovementSpeed, &AItem::ModifyMovementSpeed);
}

void AItem::Use(AAOSPlayerState* PlayerState)
{
    if (!::IsValid(PlayerState))
    {
        return;
    }

    if (PlayerState->HasAuthority())
    {
        PlayerState->SendLoadedItemsToClients();
    }
}

void AItem::BindToPlayer(AAOSCharacterBase* Character)
{
    if (::IsValid(Character))
    {
        OwnerCharacter = Character;

        Character->OnHitEventTriggered.AddDynamic(this, &AItem::OnHit);
        Character->OnAttackEventTriggered.AddDynamic(this, &AItem::OnAttack);
        Character->OnAbilityEffectsEventTriggered.AddDynamic(this, &AItem::OnAbilityEffects);
        Character->OnReceiveDamageEnteredEvent.AddDynamic(this, &AItem::OnReceiveDamageEntered);

        ApplyAbilitiesToCharacter(Character);
    }
}

void AItem::OnHit(FDamageInfomation& DamageInfomation)
{
    // Implement functionality for when character is hit
}

void AItem::OnAttack(FDamageInfomation& DamageInfomation)
{
    // Implement functionality for when character attacks
}

void AItem::OnAbilityEffects(FDamageInfomation& DamageInfomation)
{
    // Implement functionality for ability effects
}

void AItem::OnReceiveDamageEntered(bool& bResult)
{
    // Implement functionality for when character receives damage
}

void AItem::ApplyAbilitiesToCharacter(AAOSCharacterBase* Character)
{
    for (const auto& ItemAbility : Abilities)
    {
        if (AbilityFunctionMap.Contains(ItemAbility.AbilityType))
        {
            AbilityFunction Function = AbilityFunctionMap[ItemAbility.AbilityType];
            if (Function != nullptr)
            {
                UE_LOG(LogTemp, Log, TEXT("Applying ability of type %s with value %d to character %s"), *ItemAbility.AbilityTypeToString(), ItemAbility.AbilityValue, *Character->GetName());
                (this->*Function)(Character, ItemAbility.AbilityValue);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Ability type %d not found in AbilityFunctionMap"), ItemAbility.AbilityType);
        }
    }
}

void AItem::RemoveAbilitiesFromCharacter()
{
    for (const auto& ItemAbility : Abilities)
    {
        if (AbilityFunctionMap.Contains(ItemAbility.AbilityType))
        {
            AbilityFunction Function = AbilityFunctionMap[ItemAbility.AbilityType];
            if (Function != nullptr)
            {
                UE_LOG(LogTemp, Log, TEXT("Removing ability of type %s with value %d to character %s"), *ItemAbility.AbilityTypeToString(), ItemAbility.AbilityValue, *OwnerCharacter->GetName());
                (this->*Function)(OwnerCharacter.Get(), -ItemAbility.AbilityValue);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Ability type %d not found in AbilityFunctionMap"), ItemAbility.AbilityType);
        }
    }
}

template <typename T>
void AItem::ModifyStat(AAOSCharacterBase* Character, T(UStatComponent::* Getter)() const, void (UStatComponent::* Setter)(T), int32 Value)
{
    if (!::IsValid(Character))
    {
        return;
    }

    UStatComponent* PlayerStatComponent = Character->GetStatComponent();
    if (!::IsValid(PlayerStatComponent))
    {
        return;
    }

    T CurrentValue = (PlayerStatComponent->*Getter)();
    (PlayerStatComponent->*Setter)(CurrentValue + Value);
}

void AItem::ModifyMaxHealthPoints(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMaxHP, &UStatComponent::SetMaxHP, Value);
}

void AItem::ModifyMaxManaPoints(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMaxMP, &UStatComponent::SetMaxMP, Value);
}

void AItem::ModifyHealthRegeneration(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetHealthRegeneration, &UStatComponent::SetHealthRegeneration, Value);
}

void AItem::ModifyManaRegeneration(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetManaRegeneration, &UStatComponent::SetManaRegeneration, Value);
}

void AItem::ModifyAttackDamage(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAttackDamage, &UStatComponent::SetAttackDamage, Value);
}

void AItem::ModifyAbilityPower(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAbilityPower, &UStatComponent::SetAbilityPower, Value);
}

void AItem::ModifyDefensePower(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetDefensePower, &UStatComponent::SetDefensePower, Value);
}

void AItem::ModifyMagicResistance(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMagicResistance, &UStatComponent::SetMagicResistance, Value);
}

void AItem::ModifyAttackSpeed(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAttackSpeed, &UStatComponent::SetAttackSpeed, Value);
}

void AItem::ModifyAbilityHaste(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetAbilityHaste, &UStatComponent::SetAbilityHaste, Value);
}

void AItem::ModifyCriticalChance(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetCriticalChance, &UStatComponent::SetCriticalChance, Value);
}

void AItem::ModifyMovementSpeed(AAOSCharacterBase* Character, int32 Value)
{
    ModifyStat(Character, &UStatComponent::GetMovementSpeed, &UStatComponent::SetMovementSpeed, Value);
}
