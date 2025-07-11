// Copyright Abdu Inc.


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AssetDefinitionAssetInfo.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastInitialAbilitiesInfo();

	OnPlayerSpellPointsChanged.Broadcast(GetAuraPlayerState()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraAbilitySystemComponent()->AbilityStatusChangedDelegate.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag)
	{
		if (SelectedAbility.AbilityTag.MatchesTag(AbilityTag))
		{
			SelectedAbility.StatusTag = StatusTag;
			CheckAndBroadcastOnSpellGlobeSelected();
		}
			
		if (AbilitiesInfo)
		{
			FAuraAbilityInfo Info = AbilitiesInfo->FindAbilityInfoFromTag(AbilityTag);

			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	GetAuraPlayerState()->OnSpellPointsChanged.AddLambda(
	[this](int32 NewSpellPoints)
		{
			OnPlayerSpellPointsChanged.Broadcast(NewSpellPoints);
		
			CheckAndBroadcastOnSpellGlobeSelected();
		});

	GetAuraAbilitySystemComponent()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquippedToSlot);
}


void USpellMenuWidgetController::SpendPointButtonPressed()
{
	GetAuraAbilitySystemComponent()->ServerSpendSpellPoint(SelectedAbility.AbilityTag);
}

void USpellMenuWidgetController::GlobeDeselect()
{
	SelectedAbility.AbilityTag = FAuraGameplayTags::Get().Abilities_None;
	SelectedAbility.StatusTag = FAuraGameplayTags::Get().Abilities_Status_Locked;

	OnSpellGlobeSelected.Broadcast(false, false, "", "");
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	SelectedAbility.AbilityTag = AbilityTag;

	if (FGameplayAbilitySpec* AbilitySpec = GetAuraAbilitySystemComponent()->GetSpecFromAbilityTag(SelectedAbility.AbilityTag))
	{
		SelectedAbility.StatusTag = GetAuraAbilitySystemComponent()->GetStatusFromSpec(*AbilitySpec);
	}
	else
	{
		SelectedAbility.StatusTag = FAuraGameplayTags::Get().Abilities_Status_Locked;
	}

	CheckAndBroadcastOnSpellGlobeSelected();


	if (bWaitingForEquipSelection)
	{
		bWaitingForEquipSelection = false;
		
		const FGameplayTag SelectedAbilityTypeTag = AbilitiesInfo->FindAbilityInfoFromTag(AbilityTag).AbilityTypeTag;
    	StopWaitForEquipSelectionDelegate.Broadcast(SelectedAbilityTypeTag);
	}
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityTypeTag = AbilitiesInfo->FindAbilityInfoFromTag(SelectedAbility.AbilityTag).AbilityTypeTag;

	WaitForEquipSelectionDelegate.Broadcast(AbilityTypeTag);
	bWaitingForEquipSelection = true;
}


void USpellMenuWidgetController::EquippableSpellGlobePressed(const FGameplayTag& ToSlot, const FGameplayTag& AbilityTypeTag)
{
	// Fool-proofing - if we're not waiting for equip, don't do anything
	if (!bWaitingForEquipSelection)
	{
		return;
	}
	
	// Fool-proofing - if we're trying to equip an offensive ability to a passive slot, or vice versa, don't do anything
	const FGameplayTag SelectedAbilityTypeTag = AbilitiesInfo->FindAbilityInfoFromTag(SelectedAbility.AbilityTag).AbilityTypeTag;
	if (!SelectedAbilityTypeTag.MatchesTagExact(AbilityTypeTag))
	{
		return;
	}

	GetAuraAbilitySystemComponent()->ServerEquipAbilityToSlot(SelectedAbility.AbilityTag, ToSlot);
}


void USpellMenuWidgetController::CheckAndBroadcastOnSpellGlobeSelected()
{
	const int32 SpellPoints = GetAuraPlayerState()->GetSpellPoints();

	bool bSpendPointsButtonEnabled = false;
	bool bEquipButtonEnabled = false;
	
	if (SelectedAbility.StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Eligible)
		|| SelectedAbility.StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Unlocked)
		|| SelectedAbility.StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped)
		)
	{
		bSpendPointsButtonEnabled = true;
	}

	if (SelectedAbility.StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Unlocked)
		|| SelectedAbility.StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		bEquipButtonEnabled = true;
	}

	
	if (bSpendPointsButtonEnabled && SpellPoints <= 0)
	{
		bSpendPointsButtonEnabled = false;
	}


	/**
	 * Attaining Description Texts
	 */

	FString DescriptionText;
	FString NextLevelDescriptionText;

	GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(
		SelectedAbility.AbilityTag,
		DescriptionText,
		NextLevelDescriptionText
		);
	
	OnSpellGlobeSelected.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, DescriptionText, NextLevelDescriptionText);
}


void USpellMenuWidgetController::OnAbilityEquippedToSlot(const FGameplayTag& AbilityTag,
	const FGameplayTag& AbilityStatusTag, const FGameplayTag& ToSlot, const FGameplayTag& PreviousTag)
{
	bWaitingForEquipSelection = false;

	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FAuraGameplayTags::Get().Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousTag;
	LastSlotInfo.AbilityTag = FAuraGameplayTags::Get().Abilities_None;

	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo CurrentSlotInfo = AbilitiesInfo->FindAbilityInfoFromTag(AbilityTag);
	CurrentSlotInfo.StatusTag = AbilityStatusTag;
	CurrentSlotInfo.InputTag = ToSlot;

	AbilityInfoDelegate.Broadcast(CurrentSlotInfo);

	StopWaitForEquipSelectionDelegate.Broadcast(CurrentSlotInfo.AbilityTypeTag);
	
	SpellGlobeReassignedDelegate.Broadcast(CurrentSlotInfo.AbilityTag);
	GlobeDeselect();
}


