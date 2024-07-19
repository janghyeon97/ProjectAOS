// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameData.generated.h"


USTRUCT(BlueprintType)
struct FGameDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FGameDataTableRow()
		: ExperienceShareRadius(1600.f)
		, ExpShareFactorTwoPlayers(1.6236f)
		, ExpShareFactorThreePlayers(1.4157f)
		, ExpShareFactorFourPlayers(1.2494f)
		, ExpShareFactorFivePlayers(1.0f)
		, MaxAssistTime(8.0f)
	{

	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExperienceShareRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorTwoPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorThreePlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorFourPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float ExpShareFactorFivePlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float MaxAssistTime;
};


/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UGameData : public UObject
{
	GENERATED_BODY()
	
};
