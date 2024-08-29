// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UHUD.h"
#include "Game/AOSGameInstance.h"
#include "Game/AOSPlayerState.h"
#include "Game/AOSGameState.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/AbilityStatComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/ComboBoxString.h"
#include "UI/UW_HPBar.h"
#include "UI/UW_MPBar.h"
#include "UI/UW_EXPBar.h"
#include "UI/UW_AbilityLevel.h"
#include "UI/UW_Inventory.h"
#include "Item/ItemData.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/EnumAbilityID.h"

void UUHUD::NativeOnInitialized()
{

}

void UUHUD::InitializeHUD(const int32 InChampionIndex, const FName& InChampionName)
{
	ChampionIndex = InChampionIndex;
	ChampionName = InChampionName;

	// Initialize ability levels visibility
	InitializeAbilityLevels(EAbilityID::Ability_Q);
	InitializeAbilityLevels(EAbilityID::Ability_E);
	InitializeAbilityLevels(EAbilityID::Ability_R);
	InitializeAbilityLevels(EAbilityID::Ability_LMB);
	InitializeAbilityLevels(EAbilityID::Ability_RMB);

	InitializeImages();
}

void UUHUD::BindComponents(AAOSGameState* InGameState, AAOSPlayerState* InPlayerState, UStatComponent* InStatComponent, UAbilityStatComponent* InAbilityStatComponent)
{
	BindGameState(InGameState);
	BindPlayerState(InPlayerState);
	BindStatComponent(InStatComponent);
	BindAbilityStatComponent(InAbilityStatComponent);

	if (OnComponentsBindingCompleted.IsBound())
	{
		OnComponentsBindingCompleted.Broadcast();
	}
}

void UUHUD::BindStatComponent(UStatComponent* InStatComponent)
{
	if (::IsValid(InStatComponent))
	{
		StatComponent = InStatComponent;
		StatComponent->OnCurrentHPChanged.AddDynamic(HPBar, &UUW_HPBar::OnCurrentHPChanged);
		StatComponent->OnCurrentHPChanged.AddDynamic(this, &UUHUD::UpdateCurrentHPText);
		StatComponent->OnMaxHPChanged.AddDynamic(HPBar, &UUW_HPBar::OnMaxHPChanged);
		StatComponent->OnMaxHPChanged.AddDynamic(this, &UUHUD::UpdateMaxHPText);
		StatComponent->OnHPRegenChanged.AddDynamic(this, &UUHUD::UpdateHPRegenText);

		StatComponent->OnCurrentMPChanged.AddDynamic(MPBar, &UUW_MPBar::OnCurrentMPChanged);
		StatComponent->OnCurrentMPChanged.AddDynamic(this, &UUHUD::UpdateCurrentMPText);
		StatComponent->OnMaxMPChanged.AddDynamic(MPBar, &UUW_MPBar::OnMaxMPChanged);
		StatComponent->OnMaxMPChanged.AddDynamic(this, &UUHUD::UpdateMaxMPText);
		StatComponent->OnMPRegenChanged.AddDynamic(this, &UUHUD::UpdateMPRegenText);

		StatComponent->OnCurrentLevelChanged.AddDynamic(this, &UUHUD::UpdateLevelText);
		StatComponent->OnCurrentEXPChanged.AddDynamic(EXPBar, &UUW_EXPBar::OnCurrentEXPChanged);
		StatComponent->OnMaxEXPChanged.AddDynamic(EXPBar, &UUW_EXPBar::OnMaxEXPChanged);

		StatComponent->OnAttackDamageChanged.AddDynamic(this, &UUHUD::UpdateAttackDamageText);
		StatComponent->OnDefensePowerChanged.AddDynamic(this, &UUHUD::UpdateDefensePowerText);
		StatComponent->OnMagicResistanceChanged.AddDynamic(this, &UUHUD::UpdateMagicResistanceText);
		StatComponent->OnAttackSpeedChanged.AddDynamic(this, &UUHUD::UpdateAttackSpeedText);
		StatComponent->OnCriticalChanceChanged.AddDynamic(this, &UUHUD::UpdateCriticalChanceText);
		StatComponent->OnMovementSpeedChanged.AddDynamic(this, &UUHUD::UpdateMovementSpeedText);

		DecreaseHPButton->OnClicked.AddDynamic(this, &UUHUD::DecreaseHP);
		DecreaseMPButton->OnClicked.AddDynamic(this, &UUHUD::DecreaseMP);
		IncreaseLevelButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseLevel);
		IncreaseEXPButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseEXP);
		IncreaseCriticalButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseCriticalChance);
		IncreaseAttackSpeedButton->OnClicked.AddDynamic(this, &UUHUD::IncreaseAttackSpeed);
		TeamSelectionComboBox->OnSelectionChanged.AddDynamic(this, &UUHUD::ChangeTeamSide);
	}
}

void UUHUD::BindAbilityStatComponent(UAbilityStatComponent* InAbilityStatComponent)
{
	if (::IsValid(InAbilityStatComponent))
	{
		AbilityStatComponent = InAbilityStatComponent;

		AbilityStatComponent->OnAbilityQCooldownChanged.AddDynamic(this, &ThisClass::UpdateAbilityQTimerText);
		AbilityStatComponent->OnAbilityECooldownChanged.AddDynamic(this, &ThisClass::UpdateAbilityETimerText);
		AbilityStatComponent->OnAbilityRCooldownChanged.AddDynamic(this, &ThisClass::UpdateAbilityRTimerText);
		AbilityStatComponent->OnAbilityLMBCooldownChanged.AddDynamic(this, &ThisClass::UpdateAbilityLMBTimerText);
		AbilityStatComponent->OnAbilityRMBCooldownChanged.AddDynamic(this, &ThisClass::UpdateAbilityRMBTimerText);
		AbilityStatComponent->OnVisibleDescription.AddDynamic(this, &ThisClass::UpdateDescriptionText);

		AbilityStatComponent->OnAbilityLevelChanged.AddUObject(this, &ThisClass::UpdateAbilityLevel);
		AbilityStatComponent->OnAbilityVisibilityChanged.AddUObject(this, &ThisClass::UpdateAbilityVisibility);
		AbilityStatComponent->OnUpgradeWidgetVisibilityChanged.AddUObject(this, &ThisClass::UpdateUpgradeWidgetVisibility);
	}
}

void UUHUD::BindPlayerState(AAOSPlayerState* InPlayerState)
{
	if (::IsValid(InPlayerState))
	{
		PlayerState = InPlayerState;

		PlayerState->OnInventoryUpdated.AddDynamic(this, &ThisClass::UpdateInventory);
		PlayerState->OnCurrencyUpdated.AddDynamic(this, &ThisClass::UpdateCurrencyText);

		PlayerIndex = PlayerState->GetPlayerIndex();
	}
}

void UUHUD::BindGameState(AAOSGameState* InGameState)
{
	if (::IsValid(InGameState))
	{
		GameState = InGameState;

		GameState->OnRespawnTimerUpdated.AddDynamic(this, &ThisClass::UpdateRespawnTimeText);
	}
}


void UUHUD::InitializeAbilityLevels(EAbilityID AbilityID)
{
	TArray<UWidgetSwitcher*>* AbilitySwitchers = nullptr;
	int MaxLevel = AbilityStatComponent->GetAbilityInfomation(AbilityID).MaxLevel;

	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		AbilitySwitchers = &Ability_Q_Level->Switchers;
		break;
	case EAbilityID::Ability_E:
		AbilitySwitchers = &Ability_E_Level->Switchers;
		break;
	case EAbilityID::Ability_R:
		AbilitySwitchers = &Ability_R_Level->Switchers;
		break;
	case EAbilityID::Ability_LMB:
		AbilitySwitchers = &Ability_LMB_Level->Switchers;
		break;
	case EAbilityID::Ability_RMB:
		AbilitySwitchers = &Ability_RMB_Level->Switchers;
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid AbilityID: %d"), static_cast<int32>(AbilityID));
		return;
	}

	if (AbilitySwitchers)
	{
		for (int i = 0; i < MaxLevel; i++)
		{
			if (AbilitySwitchers->IsValidIndex(i))
			{
				(*AbilitySwitchers)[i]->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
	}
}

void UUHUD::InitializeImages()
{
	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(GetWorld()->GetGameInstance());
	if (::IsValid(GameInstance) == false)
	{
		return;
	}

	if (GameInstance->GetCampionsListTableRow(ChampionName) == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UHUD::InitializeAbilityImage] CampionsList %s Index is not vaild"), *ChampionName.ToString());
		return;
	}

	UpdateChampionText(ChampionName);

	UTexture* Ability_Q_Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL,
		*FString::Printf(TEXT("/Game/ProjectAOS/Characters/%s/Images/T_Ability_Q.T_Ability_Q"), *ChampionName.ToString())));

	UTexture* Ability_E_Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL,
		*FString::Printf(TEXT("/Game/ProjectAOS/Characters/%s/Images/T_Ability_E.T_Ability_E"), *ChampionName.ToString())));

	UTexture* Ability_R_Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL,
		*FString::Printf(TEXT("/Game/ProjectAOS/Characters/%s/Images/T_Ability_R.T_Ability_R"), *ChampionName.ToString())));

	UTexture* Ability_LMB_Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL,
		*FString::Printf(TEXT("/Game/ProjectAOS/Characters/%s/Images/T_Primary_LMB.T_Primary_LMB"), *ChampionName.ToString())));

	UTexture* Ability_RMB_Texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL,
		*FString::Printf(TEXT("/Game/ProjectAOS/Characters/%s/Images/T_Primary_RMB.T_Primary_RMB"), *ChampionName.ToString())));


	MaterialRef.Empty();

	MaterialRef.Add(Ability_Q_Image->GetDynamicMaterial());
	MaterialRef.Add(Ability_E_Image->GetDynamicMaterial());
	MaterialRef.Add(Ability_R_Image->GetDynamicMaterial());
	MaterialRef.Add(Ability_LMB_Image->GetDynamicMaterial());
	MaterialRef.Add(Ability_RMB_Image->GetDynamicMaterial());

	if (::IsValid(MaterialRef[0]))
	{
		MaterialRef[0]->SetTextureParameterValue(FName("Tex"), Ability_Q_Texture);
		MaterialRef[0]->SetScalarParameterValue(FName("Percent"), 0.0f);
	}
	if (::IsValid(MaterialRef[1]))
	{
		MaterialRef[1]->SetTextureParameterValue(FName("Tex"), Ability_E_Texture);
		MaterialRef[1]->SetScalarParameterValue(FName("Percent"), 0.0f);
	}
	if (::IsValid(MaterialRef[2]))
	{
		MaterialRef[2]->SetTextureParameterValue(FName("Tex"), Ability_R_Texture);
		MaterialRef[2]->SetScalarParameterValue(FName("Percent"), 0.0f);
	}
	if (::IsValid(MaterialRef[3]))
	{
		MaterialRef[3]->SetTextureParameterValue(FName("Tex"), Ability_LMB_Texture);
		MaterialRef[3]->SetScalarParameterValue(FName("Percent"), 0.0f);
	}
	if (::IsValid(MaterialRef[4]))
	{
		MaterialRef[4]->SetTextureParameterValue(FName("Tex"), Ability_RMB_Texture);
		MaterialRef[4]->SetScalarParameterValue(FName("Percent"), 0.0f);
	}


	CharacterImageMaterialRef = ChampionImage->GetDynamicMaterial();
	if (CharacterImageMaterialRef)
	{
		UTexture* Texture = GameInstance->GetCampionsListTableRow(ChampionName)->ChampionImage;
		if (::IsValid(Texture))
		{
			CharacterImageMaterialRef->SetTextureParameterValue(FName("Image"), Texture);
		}
	}

	RespawnTimeImageMaterialRef = RespawnTimeImage->GetDynamicMaterial();

	UTexture* RespawnTimeImageTexture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL, TEXT("/Game/ProjectAOS/UI/Image/Image_Alpha95.Image_Alpha95")));
	if (RespawnTimeImageTexture)
	{
		RespawnTimeImageMaterialRef->SetTextureParameterValue(FName("Image"), RespawnTimeImageTexture);
	}
}

void UUHUD::DecreaseHP()
{
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		OwningCharacter->DecreaseHP_Server();
	}
}

void UUHUD::DecreaseMP()
{
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		OwningCharacter->DecreaseMP_Server();
	}
}

void UUHUD::IncreaseEXP()
{
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		OwningCharacter->IncreaseEXP_Server();
	}
}

void UUHUD::IncreaseLevel()
{
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		OwningCharacter->IncreaseLevel_Server();
	}
}

void UUHUD::IncreaseCriticalChance()
{
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		OwningCharacter->IncreaseCriticalChance_Server();
	}
}

void UUHUD::IncreaseAttackSpeed()
{
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		OwningCharacter->IncreaseAttackSpeed_Server();
	}
}

void UUHUD::ChangeTeamSide(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	AAOSCharacterBase* OwningCharacter = Cast<AAOSCharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		if (SelectedItem.Equals("Blue"))
		{
			OwningCharacter->ChangeTeamSide_Server(ETeamSideBase::Blue);
		}
		else if (SelectedItem.Equals("Red"))
		{
			OwningCharacter->ChangeTeamSide_Server(ETeamSideBase::Red);
		}
		else
		{
			UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Team change failed. Please choose between blue and red."), true, true, FLinearColor::Red, 2.0f, FName("Name_None"));
		}
	}
}

void UUHUD::UpdateAbilityVisibility(EAbilityID AbilityID, bool InVisibility)
{
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		if (InVisibility) { Ability_Q_Border->SetVisibility(ESlateVisibility::Hidden); }
		else { Ability_Q_Border->SetVisibility(ESlateVisibility::Visible); }
		break;

	case EAbilityID::Ability_E:
		if (InVisibility) { Ability_E_Border->SetVisibility(ESlateVisibility::Hidden); }
		else { Ability_E_Border->SetVisibility(ESlateVisibility::Visible); }
		break;

	case EAbilityID::Ability_R:
		if (InVisibility) { Ability_R_Border->SetVisibility(ESlateVisibility::Hidden); }
		else { Ability_R_Border->SetVisibility(ESlateVisibility::Visible); }
		break;

	case EAbilityID::Ability_LMB:
		if (InVisibility) { Ability_LMB_Border->SetVisibility(ESlateVisibility::Hidden); }
		else { Ability_LMB_Border->SetVisibility(ESlateVisibility::Visible); }
		break;

	case EAbilityID::Ability_RMB:
		if (InVisibility) { Ability_RMB_Border->SetVisibility(ESlateVisibility::Hidden); }
		else { Ability_RMB_Border->SetVisibility(ESlateVisibility::Visible); }
		break;
	}
}

void UUHUD::UpdateUpgradeWidgetVisibility(EAbilityID AbilityID, bool InVisibility)
{
	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		if (InVisibility) { LevelUp_Q_Image->SetVisibility(ESlateVisibility::Visible); }
		else { LevelUp_Q_Image->SetVisibility(ESlateVisibility::Hidden); }
		break;

	case EAbilityID::Ability_E:
		if (InVisibility) { LevelUp_E_Image->SetVisibility(ESlateVisibility::Visible); }
		else { LevelUp_E_Image->SetVisibility(ESlateVisibility::Hidden); }
		break;

	case EAbilityID::Ability_R:
		if (InVisibility) { LevelUp_R_Image->SetVisibility(ESlateVisibility::Visible); }
		else { LevelUp_R_Image->SetVisibility(ESlateVisibility::Hidden); }
		break;

	case EAbilityID::Ability_LMB:
		if (InVisibility) { LevelUp_LMB_Image->SetVisibility(ESlateVisibility::Visible); }
		else { LevelUp_LMB_Image->SetVisibility(ESlateVisibility::Hidden); }
		break;

	case EAbilityID::Ability_RMB:
		if (InVisibility) { LevelUp_RMB_Image->SetVisibility(ESlateVisibility::Visible); }
		else { LevelUp_RMB_Image->SetVisibility(ESlateVisibility::Hidden); }
		break;
	}
}

void UUHUD::InitializeAbilityLevel(EAbilityID AbilityID)
{
	TArray<UWidgetSwitcher*>* AbilitySwitchers = nullptr;

	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		AbilitySwitchers = &Ability_Q_Level->Switchers;
		break;
	case EAbilityID::Ability_E:
		AbilitySwitchers = &Ability_E_Level->Switchers;
		break;
	case EAbilityID::Ability_R:
		AbilitySwitchers = &Ability_R_Level->Switchers;
		break;
	case EAbilityID::Ability_LMB:
		AbilitySwitchers = &Ability_LMB_Level->Switchers;
		break;
	case EAbilityID::Ability_RMB:
		AbilitySwitchers = &Ability_RMB_Level->Switchers;
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid AbilityID"));
		return;
	}

	if (AbilitySwitchers)
	{
		for (auto& Switcher : *AbilitySwitchers)
		{
			if (::IsValid(Switcher))
			{
				Switcher->SetActiveWidgetIndex(0);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid AbilityID: %d"), static_cast<int32>(AbilityID));
	}
}

void UUHUD::UpdateAbilityLevel(EAbilityID AbilityID, int InLevel)
{
	InitializeAbilityLevel(AbilityID);

	TArray<UWidgetSwitcher*>* AbilitySwitchers = nullptr;

	switch (AbilityID)
	{
	case EAbilityID::Ability_Q:
		AbilitySwitchers = &Ability_Q_Level->Switchers;
		break;
	case EAbilityID::Ability_E:
		AbilitySwitchers = &Ability_E_Level->Switchers;
		break;
	case EAbilityID::Ability_R:
		AbilitySwitchers = &Ability_R_Level->Switchers;
		break;
	case EAbilityID::Ability_LMB:
		AbilitySwitchers = &Ability_LMB_Level->Switchers;
		break;
	case EAbilityID::Ability_RMB:
		AbilitySwitchers = &Ability_RMB_Level->Switchers;
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid AbilityID"));
		return;
	}

	if (AbilitySwitchers != nullptr)
	{
		for (int i = 0; i < InLevel; i++)
		{
			if (AbilitySwitchers->IsValidIndex(i))
			{
				UWidgetSwitcher* Switcher = (*AbilitySwitchers)[i];
				if (::IsValid(Switcher))
				{
					Switcher->SetActiveWidgetIndex(1);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Switcher index %d is out of range for ability %d"), i, static_cast<int32>(AbilityID));
			}
		}
	}
}


void UUHUD::UpdateAbilityQTimerText(float MaxCooldown, float CurrentCooldown)
{
	FString CooldownString;

	if (CurrentCooldown <= 1.0f) CooldownString = FString::Printf(TEXT("%.1f"), CurrentCooldown);
	else CooldownString = FString::Printf(TEXT("%d"), (int)CurrentCooldown);

	Ability_Q_CooldownText->SetVisibility(ESlateVisibility::Visible);
	Ability_Q_CooldownText->SetText(FText::FromString(CooldownString));

	if (CurrentCooldown <= 0)
	{
		Ability_Q_CooldownText->SetVisibility(ESlateVisibility::Hidden);
		CurrentCooldown = 0;
	}
	MaterialRef[0]->SetScalarParameterValue(FName("Percent"), 1 - CurrentCooldown / MaxCooldown);
}

void UUHUD::UpdateAbilityETimerText(float MaxCooldown, float CurrentCooldown)
{
	FString CooldownString;

	if (CurrentCooldown <= 1.0f) CooldownString = FString::Printf(TEXT("%.1f"), CurrentCooldown);
	else CooldownString = FString::Printf(TEXT("%d"), (int)CurrentCooldown);

	Ability_E_CooldownText->SetVisibility(ESlateVisibility::Visible);
	Ability_E_CooldownText->SetText(FText::FromString(CooldownString));

	if (CurrentCooldown <= 0)
	{
		Ability_E_CooldownText->SetVisibility(ESlateVisibility::Hidden);
		CurrentCooldown = 0;
	}
	MaterialRef[1]->SetScalarParameterValue(FName("Percent"), 1 - CurrentCooldown / MaxCooldown);
}

void UUHUD::UpdateAbilityRTimerText(float MaxCooldown, float CurrentCooldown)
{
	FString CooldownString;

	if (CurrentCooldown <= 1.0f) CooldownString = FString::Printf(TEXT("%.1f"), CurrentCooldown);
	else CooldownString = FString::Printf(TEXT("%d"), (int)CurrentCooldown);

	Ability_R_CooldownText->SetVisibility(ESlateVisibility::Visible);
	Ability_R_CooldownText->SetText(FText::FromString(CooldownString));

	if (CurrentCooldown <= 0)
	{
		Ability_R_CooldownText->SetVisibility(ESlateVisibility::Hidden);
		CurrentCooldown = 0;
	}
	MaterialRef[2]->SetScalarParameterValue(FName("Percent"), 1 - CurrentCooldown / MaxCooldown);
}

void UUHUD::UpdateAbilityLMBTimerText(float MaxCooldown, float CurrentCooldown)
{
	FString CooldownString;

	if (CurrentCooldown <= 1.0f) CooldownString = FString::Printf(TEXT("%.1f"), CurrentCooldown);
	else CooldownString = FString::Printf(TEXT("%d"), (int)CurrentCooldown);

	Ability_LMB_CooldownText->SetVisibility(ESlateVisibility::Visible);
	Ability_LMB_CooldownText->SetText(FText::FromString(CooldownString));

	if (CurrentCooldown <= 0)
	{
		Ability_LMB_CooldownText->SetVisibility(ESlateVisibility::Hidden);
		CurrentCooldown = 0;
	}
	MaterialRef[3]->SetScalarParameterValue(FName("Percent"), 1 - CurrentCooldown / MaxCooldown);
}

void UUHUD::UpdateAbilityRMBTimerText(float MaxCooldown, float CurrentCooldown)
{
	FString CooldownString;

	if (CurrentCooldown <= 1.0f) CooldownString = FString::Printf(TEXT("%.1f"), CurrentCooldown);
	else CooldownString = FString::Printf(TEXT("%d"), (int)CurrentCooldown);

	Ability_RMB_CooldownText->SetVisibility(ESlateVisibility::Visible);
	Ability_RMB_CooldownText->SetText(FText::FromString(CooldownString));

	if (CurrentCooldown <= 0)
	{
		Ability_RMB_CooldownText->SetVisibility(ESlateVisibility::Hidden);
		CurrentCooldown = 0;
	}
	MaterialRef[4]->SetScalarParameterValue(FName("Percent"), 1 - CurrentCooldown / MaxCooldown);
}

void UUHUD::UpdateMaxHPText(float InOldMaxHP, float InNewMaxHP)
{
	FString MaxHPString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewMaxHP));
	
	MaxHPText->SetText(FText::FromString(MaxHPString));
}

void UUHUD::UpdateCurrentHPText(float InOldHP, float InNewHP)
{
	FString CurrentHPString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewHP));

	CurrentHPText->SetText(FText::FromString(CurrentHPString));
}

void UUHUD::UpdateAttackDamageText(float InOldAD, float InNewAD)
{
	FString AttackDamageString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewAD));

	AttackDamageText->SetText(FText::FromString(AttackDamageString));
}

void UUHUD::UpdateDefensePowerText(float InOldDP, float InNewDP)
{
	FString DefensePowerString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewDP));

	DefensePowerText->SetText(FText::FromString(DefensePowerString));
}

void UUHUD::UpdateMagicResistanceText(float InOldMR, float InNewMR)
{
	FString MagicResistanceString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewMR));

	MagicResistanceText->SetText(FText::FromString(MagicResistanceString));
}

void UUHUD::UpdateAttackSpeedText(float InOldAS, float InNewAS)
{
	FString AttackSpeedString = FString::Printf(TEXT("%.2f"), InNewAS);

	AttackSpeedText->SetText(FText::FromString(AttackSpeedString));
}

void UUHUD::UpdateCriticalChanceText(int32 InOldCC, int32 InNewCC)
{
	FString CriticalChanceString = FString::Printf(TEXT("%d"), InNewCC);

	CriticalChanceText->SetText(FText::FromString(CriticalChanceString));
}

void UUHUD::UpdateMovementSpeedText(float InOldMS, float InNewMS)
{
	FString MovementSpeedString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewMS));

	MovementSpeedText->SetText(FText::FromString(MovementSpeedString));
}

void UUHUD::UpdateHPRegenText(float InOldHPRegen, float InNewHPRegen)
{
	FString HPRegenString = FString::Printf(TEXT("%.1f"), InNewHPRegen);
	
	HPRegenText->SetText(FText::FromString(HPRegenString));
}

void UUHUD::UpdateMaxMPText(float InOldMaxMP, float InNewMaxMP)
{
	FString MaxMPString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewMaxMP));

	MaxMPText->SetText(FText::FromString(MaxMPString));
}

void UUHUD::UpdateCurrentMPText(float InOldMP, float InNewMP)
{
	FString CurrentMPString = FString::Printf(TEXT("%d"), FMath::CeilToInt(InNewMP));

	CurrentMPText->SetText(FText::FromString(CurrentMPString));
}

void UUHUD::UpdateMPRegenText(float InOldMPRegen, float InNewMPRegen)
{
	FString MPRegenString = FString::Printf(TEXT("%.1f"), InNewMPRegen);

	MPRegenText->SetText(FText::FromString(MPRegenString));
}

void UUHUD::UpdateChampionText(const FName& NewString)
{
	ChampionNameText->SetText(FText::FromName(NewString));
}

void UUHUD::UpdateInventory(const TArray<FItemInformation>& Items)
{
	if (!PlayerState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[UUHUD::UpdateInventory] PlayerState is nullptr"));
		return;
	}

	if (Items.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UUHUD::UpdateInventory] No items in inventory"));
		return;
	}

	int32 Index = 0;
	for (const auto& Item: Items)
	{
		if (!Item.IsEmpty() && Item.Icon)
		{
			Inventory->UpdateItemImage(Item.Icon, Index);
			Inventory->UpdateItemCountText(Item.CurrentStack, Index);
		}
		else
		{
			Inventory->UpdateItemImage(nullptr, Index);
		}

		Index++;
	}
}

void UUHUD::UpdateCurrencyText(const int32 NewCurrency)
{
	FString CurrencyString = FString::Printf(TEXT("%d"), NewCurrency);
	Inventory->UpdateCurrencyText(FText::FromString(CurrencyString));
}

void UUHUD::UpdateLevelText(int32 InOldLevel, int32 InNewLevel)
{
	FString LevelString = FString::Printf(TEXT("%d"), InNewLevel);

	LevelText->SetText(FText::FromString(LevelString));
}

void UUHUD::UpdateDescriptionText(FName InString)
{
	FTimerHandle NewTimerHandle;

	DescriptionText->SetVisibility(ESlateVisibility::Visible);
	DescriptionText->SetText(FText::FromString(InString.ToString()));

	GetWorld()->GetTimerManager().SetTimer(
		NewTimerHandle, [&]()
		{
			DescriptionText->SetVisibility(ESlateVisibility::Hidden);
		},
		0.1f,
		false,
		2.0f
	);
}

void UUHUD::UpdateRespawnTimeText(int32 InPlayerIndex, float RemainingTime)
{
	if (PlayerIndex == InPlayerIndex)
	{
		RespawnTimeImage->SetVisibility(ESlateVisibility::Visible);
		RespawnTimeText->SetVisibility(ESlateVisibility::Visible);

		FString RemainingTimeString = FString::Printf(TEXT("%d"), FMath::CeilToInt(RemainingTime));
		RespawnTimeText->SetText(FText::FromString(RemainingTimeString));

		if (RemainingTime <= 0.1f)
		{
			RespawnTimeImage->SetVisibility(ESlateVisibility::Hidden);
			RespawnTimeText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{

	}
}
