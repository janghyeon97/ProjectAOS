// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/EnumMinionType.h"
#include "MinionData.generated.h"



USTRUCT(BlueprintType)
struct FMinionDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMinionDataTableRow()
		: MinionType(EMinionType::None)
		, StatTable(nullptr)
		, AbilityStatTable(nullptr)
		, SkeletalMesh_Down(nullptr)
		, SkeletalMesh_Dusk(nullptr)
		, MinionClass(nullptr)
		, Montages(TMap<FString, UAnimMontage*>())
	{

	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMinionType MinionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExpBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* StatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* AbilityStatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh_Down;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh_Dusk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* MinionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, UAnimMontage*> Montages;
};

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UMinionData : public UObject
{
	GENERATED_BODY()
	
};
