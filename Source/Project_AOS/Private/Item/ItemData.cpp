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

    // 색상 구분자 처리
    Result = ApplyColorTags(Result);

    // StatComponent 값 구분자 처리
    Result = ReplaceStatTags(Result, StatComponent);

    // 수식 구분자 처리
    Result = ReplaceCalcTags(Result, StatComponent);

    return Result;
}



/**
 * ApplyColorTags 함수는 주어진 텍스트에서 색상 구분자를 찾아
 * HTML 태그 형식으로 대체합니다.
 *
 * @param Text: 입력 텍스트입니다.
 * @return 색상 태그가 적용된 텍스트입니다.
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
 * ReplaceStatTags 함수는 주어진 텍스트에서 {stat=...} 형식의 스탯 구분자를 찾아
 * 해당 구분자를 StatComponent의 값으로 대체합니다.
 *
 * @param Text: 입력 텍스트입니다.
 * @param StatComponent: 스탯 값을 제공하는 컴포넌트입니다.
 * @return 변환된 텍스트입니다.
 *
 * 주요 작업:
 * 1. StatComponent가 유효한지 확인합니다.
 * 2. StatGetters 맵을 순회하여 각 스탯 구분자를 StatComponent의 값으로 대체합니다.
 * 3. StatGettersInt 맵을 순회하여 각 스탯 구분자를 StatComponent의 정수 값으로 대체합니다.
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
 * ReplaceCalcTags 함수는 주어진 텍스트에서 {calc=...} 형식의 수식 구분자를 찾아 평가된 결과로 대체합니다.
 * - Text: 입력 텍스트입니다.
 * - StatComponent: 스탯 값을 제공하는 컴포넌트입니다.
 *
 * 주요 작업:
 * 1. 수식 구분자를 찾아 평가합니다.
 * 2. 평가된 결과로 수식 구분자를 대체합니다.
 * 3. StatComponent에서 스탯 값을 가져와 변수 맵에 추가합니다.
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
            FString CalcTag = Result.Mid(StartIndex + 6, EndIndex - StartIndex - 6); // "{calc=" 이후와 "}" 이전의 문자열
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
