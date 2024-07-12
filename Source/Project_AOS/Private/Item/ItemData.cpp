// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemData.h"
#include "Components/StatComponent.h"
#include "Plugins/ExpressionEvaluator.h"

FItemInformation::FItemInformation()
    : ItemID(0)
    , Name(TEXT("Default Name"))
    , Price(0)
    , Description(TEXT("Default Description"))
    , Icon(nullptr)
    , ItemStackLimit(0)
    , CurrentStack(0)
    , MaxPossessQuantity(1)
    , Classification(EClassification::None)
    , Abilities(TArray<FItemAbility>())
    , RequiredItems(TArray<int>())
{
    StatGetters = {
        {TEXT("MaxHealth"), &UStatComponent::GetMaxHP},
        {TEXT("CurrentHealth"), &UStatComponent::GetCurrentHP},
        {TEXT("MaxMana"), &UStatComponent::GetMaxMP},
        {TEXT("CurrentMana"), &UStatComponent::GetCurrentMP},
        {TEXT("HealthRegeneration"), &UStatComponent::GetHealthRegeneration},
        {TEXT("ManaRegeneration"), &UStatComponent::GetManaRegeneration},
        {TEXT("AttackDamage"), &UStatComponent::GetAttackDamage},
        {TEXT("AbilityPower"), &UStatComponent::GetAbilityPower},
        {TEXT("DefensePower"), &UStatComponent::GetDefensePower},
        {TEXT("MagicResistance"), &UStatComponent::GetMagicResistance},
        {TEXT("AttackSpeed"), &UStatComponent::GetAttackSpeed},
        {TEXT("MovementSpeed"), &UStatComponent::GetMovementSpeed}
    };

    StatGettersInt = {
        {TEXT("AbilityHaste"), &UStatComponent::GetAbilityHaste},
        {TEXT("CriticalChance"), &UStatComponent::GetCriticalChance}
    };
}

FString FItemInformation::ClassificationToString() const
{
    switch (Classification)
    {
    case EClassification::None:                 return TEXT("None");
    case EClassification::Starter:              return TEXT("Starter");
    case EClassification::Potions:              return TEXT("Potions");
    case EClassification::Consumables:          return TEXT("Consumables");
    case EClassification::Trinkets:             return TEXT("Trinkets");
    case EClassification::Boots:                return TEXT("Boots");
    case EClassification::Basic:                return TEXT("Basic");
    case EClassification::Epic:                 return TEXT("Epic");
    case EClassification::Legendary:            return TEXT("Legendary");
    case EClassification::Distributed:          return TEXT("Distributed");
    case EClassification::ChampionExclusive:    return TEXT("Champion Exclusive");
    default:                                    return TEXT("Unknown");
    }
}


FString FItemInformation::ConvertToRichText(UStatComponent* StatComponent) const
{
    FString Result = Description;

    // ���� ������ ó��
    Result = ApplyColorTags(Result);

    // StatComponent �� ������ ó��
    Result = ReplaceStatTags(Result, StatComponent);

    // ���� ������ ó��
    Result = ReplaceCalcTags(Result, StatComponent);

    return Result;
}



/**
 * ApplyColorTags �Լ��� �־��� �ؽ�Ʈ���� ���� �����ڸ� ã��
 * HTML �±� �������� ��ü�մϴ�.
 *
 * @param Text: �Է� �ؽ�Ʈ�Դϴ�.
 * @return ���� �±װ� ����� �ؽ�Ʈ�Դϴ�.
 */
FString FItemInformation::ApplyColorTags(const FString& Text) const
{
    FString Result = Text;
    Result = Result.Replace(TEXT("{color=red}"), TEXT("<red>"));
    Result = Result.Replace(TEXT("{color=green}"), TEXT("<green>"));
    Result = Result.Replace(TEXT("{color=blue}"), TEXT("<blue>"));
    Result = Result.Replace(TEXT("{/color}"), TEXT("</>"));
    return Result;
}


/**
 * ReplaceStatTags �Լ��� �־��� �ؽ�Ʈ���� {stat=...} ������ ���� �����ڸ� ã��
 * �ش� �����ڸ� StatComponent�� ������ ��ü�մϴ�.
 *
 * @param Text: �Է� �ؽ�Ʈ�Դϴ�.
 * @param StatComponent: ���� ���� �����ϴ� ������Ʈ�Դϴ�.
 * @return ��ȯ�� �ؽ�Ʈ�Դϴ�.
 *
 * �ֿ� �۾�:
 * 1. StatComponent�� ��ȿ���� Ȯ���մϴ�.
 * 2. StatGetters ���� ��ȸ�Ͽ� �� ���� �����ڸ� StatComponent�� ������ ��ü�մϴ�.
 * 3. StatGettersInt ���� ��ȸ�Ͽ� �� ���� �����ڸ� StatComponent�� ���� ������ ��ü�մϴ�.
 */
FString FItemInformation::ReplaceStatTags(const FString& Text, UStatComponent* StatComponent) const
{
    FString Result = Text;

    if (StatComponent)
    {
        for (const auto& Pair : StatGetters)
        {
            FString Tag = FString::Printf(TEXT("{stat=%s}"), *Pair.Key);
            double Value = (StatComponent->*(Pair.Value))();
            Result = Result.Replace(*Tag, *FString::SanitizeFloat(Value));
        }

        for (const auto& Pair : StatGettersInt)
        {
            FString Tag = FString::Printf(TEXT("{stat=%s}"), *Pair.Key);
            int32 Value = (StatComponent->*(Pair.Value))();
            Result = Result.Replace(*Tag, *FString::FromInt(Value));
        }
    }

    return Result;
}


/**
 * ReplaceCalcTags �Լ��� �־��� �ؽ�Ʈ���� {calc=...} ������ ���� �����ڸ� ã�� �򰡵� ����� ��ü�մϴ�.
 * - Text: �Է� �ؽ�Ʈ�Դϴ�.
 * - StatComponent: ���� ���� �����ϴ� ������Ʈ�Դϴ�.
 *
 * �ֿ� �۾�:
 * 1. ���� �����ڸ� ã�� ���մϴ�.
 * 2. �򰡵� ����� ���� �����ڸ� ��ü�մϴ�.
 * 3. StatComponent���� ���� ���� ������ ���� �ʿ� �߰��մϴ�.
 */
FString FItemInformation::ReplaceCalcTags(const FString& Text, UStatComponent* StatComponent) const
{
    FString Result = Text;
    ExpressionEvaluator Evaluator;

    if (!StatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FItemInformation::ReplaceCalcTags] Invalid StatComponent."));
        return Result;
    }

    std::unordered_map<std::string, double> Variables;

    for (const auto& Pair : StatGetters)
    {
        Variables[std::string(TCHAR_TO_UTF8(*Pair.Key))] = (StatComponent->*(Pair.Value))();
    }

    for (const auto& Pair : StatGettersInt)
    {
        Variables[std::string(TCHAR_TO_UTF8(*Pair.Key))] = static_cast<double>((StatComponent->*(Pair.Value))());
    }

    int32 StartIndex = 0;
    while (true)
    {
        StartIndex = Result.Find(TEXT("{calc="), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex);
        if (StartIndex == INDEX_NONE)
        {
            break;
        }

        int32 EndIndex = 0;
        if (Result.FindChar('}', EndIndex))
        {
            FString CalcTag = Result.Mid(StartIndex + 6, EndIndex - StartIndex - 6); // "{calc=" ���Ŀ� "}" ������ ���ڿ�
            std::string CalcStr = TCHAR_TO_UTF8(*CalcTag);

            try
            {
                double CalcResult = Evaluator.Evaluate(CalcStr, Variables);
                Result = Result.Replace(*FString::Printf(TEXT("{calc=%s}"), *CalcTag), *FString::SanitizeFloat(CalcResult));
            }
            catch (const std::runtime_error& e)
            {
                UE_LOG(LogTemp, Warning, TEXT("[FItemInformation::ReplaceCalcTags] Failed to parse expression: %s"), *FString(e.what()));
            }

            StartIndex = EndIndex + 1;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[FItemInformation::ReplaceCalcTags] Mismatched braces in CalcTag."));
            break;
        }
    }

    return Result;
}
