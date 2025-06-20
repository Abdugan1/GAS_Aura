// Copyright Abdu Inc.


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
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
		});
}


void USpellMenuWidgetController::SpendPointButtonPressed(const FGameplayTag& AbilityTag)
{
	GetAuraAbilitySystemComponent()->ServerSpendSpellPoint(AbilityTag);
}


