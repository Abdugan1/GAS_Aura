// Copyright Abdu Inc.


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastInitialAbilitiesInfo();

	AAuraPlayerState* AuraPS = CastChecked<AAuraPlayerState>(PlayerState);
	OnPlayerSpellPointsChanged.Broadcast(AuraPS->GetSpellPoints());
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

	AAuraPlayerState* AuraPS = CastChecked<AAuraPlayerState>(PlayerState);

	AuraPS->OnSpellPointsChanged.AddLambda(
	[this](int32 NewSpellPoints)
		{
			OnPlayerSpellPointsChanged.Broadcast(NewSpellPoints);
		});
}
