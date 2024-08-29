// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/ItemAbilityData.h"
#include "Structs/EnumClassification.h"
#include "Structs/EnumItemAbility.h"
#include "ItemData.generated.h"

class UStatComponent;

USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FItemTableRow()
		: ItemID(0)
		, Name(FString())
		, Price(0)
		, Description(FString())
		, Icon(nullptr)
		, ItemStackLimit(0)
		, MaxPossessQuantity(0)
		, Classification(EClassification::None)
		, Abilities(TArray<FItemAbility>())
		, RequiredItems(TArray<int>())
		, ItemClass(nullptr)
	{

	}

	bool IsEmpty() const
	{
		return ItemID == 0;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemStackLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxPossessQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EClassification Classification;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FItemAbility> Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<int> RequiredItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class AItem> ItemClass;
};

USTRUCT(BlueprintType)
struct FItemInformation
{
	GENERATED_BODY()

public:
	FItemInformation();
	~FItemInformation();

	using StatGetter = float(UStatComponent::*)() const;

	bool IsEmpty() const
	{
		return ItemID == 0;
	}
	FString ClassificationToString() const;
	FString ConvertToRichText(UStatComponent* StatComponent) const;

private:
	FString ApplyColorTags(const FString& Text) const;
	FString ReplaceStatTags(const FString& Text, UStatComponent* StatComponent) const;
	FString ReplaceCalcTags(const FString& Text, UStatComponent* StatComponent) const;

	TMap<FString, float(UStatComponent::*)() const> StatGetters;
	TMap<FString, int32(UStatComponent::*)() const> StatGettersInt;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	int32 ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	int32 Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	UTexture* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	int32 ItemStackLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	int32 CurrentStack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemInfo")
	int32 MaxPossessQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EClassification Classification;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FItemAbility> Abilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<int> RequiredItems;
};


/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UItemData : public UObject
{
	GENERATED_BODY()
	
};
