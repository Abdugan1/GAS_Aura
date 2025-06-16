// Copyright Abdu Inc.


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastInitialAbilitiesInfo();
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);

	AuraASC->AbilityStatusChangedDelegate.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag)
	{
		if (AbilitiesInfo)
		{
			FAuraAbilityInfo Info = AbilitiesInfo->FindAbilityInfoFromTag(AbilityTag);

			// NOTE: For some reason, Stephen changes Info.StatusTag = StatusTag here. But I'm changing it inside ASC's UpdateAbilityStatuses
			AbilityInfoDelegate.Broadcast(Info);
		}
	});
}
