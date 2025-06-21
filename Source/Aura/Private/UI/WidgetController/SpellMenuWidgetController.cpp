// Copyright Abdu Inc.


#include "UI/WidgetController/SpellMenuWidgetController.h"

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
}


void USpellMenuWidgetController::SpendPointButtonPressed()
{
	GetAuraAbilitySystemComponent()->ServerSpendSpellPoint(SelectedAbility.AbilityTag);
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
	
	// if (FGameplayAbilitySpec* AbilitySpec = GetAuraAbilitySystemComponent()->GetSpecFromAbilityTag(SelectedAbility.AbilityTag))
	// {
	// 	UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability);
	//
	// 	DescriptionText = AuraAbility->GetDescription(AbilitySpec->Level);
	// 	NextLevelDescriptionText = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
	// }
	// else
	// {
	// 	const UAbilityInfo* Abilities = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAbilitySystemComponent()->GetAvatarActor());
	// 	const FAuraAbilityInfo AbilityInfo = Abilities->FindAbilityInfoFromTag(SelectedAbility.AbilityTag);
	// 	
	// 	DescriptionText= UAuraGameplayAbility::GetLockedDescription(AbilityInfo.LevelUpRequirement);
	// }
	
	
	OnSpellGlobeSelected.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, DescriptionText, NextLevelDescriptionText);
}


