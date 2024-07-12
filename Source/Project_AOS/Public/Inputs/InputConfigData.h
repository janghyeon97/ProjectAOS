// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputConfigData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AOS_API UInputConfigData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Ability_Q_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Ability_E_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Ability_R_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Ability_LMB_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Ability_RMB_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> SwitchInputMappingContext_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Upgrade_Ability_Q_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Upgrade_Ability_E_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Upgrade_Ability_R_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Upgrade_Ability_LMB_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> Upgrade_Ability_RMB_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> UseItem_1_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> UseItem_2_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> UseItem_3_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> UseItem_4_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> UseItem_5_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> UseItem_6_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> ToggleItemShop_Action;

	UPROPERTY(EditAnyWhere, BlueprintReadOnly)
	TObjectPtr<class UInputAction> CallAFunctionAction;
};
