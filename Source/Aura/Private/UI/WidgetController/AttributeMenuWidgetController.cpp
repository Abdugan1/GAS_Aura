// Copyright Abdu Inc.


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/AuraPlayerState.h"


void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	check(AttributeInfo);

	// TODO! Why can I not use values? I guess I can here. But in the BindCallbacks...
	for (FAuraAttributeInfo& Info : AttributeInfo->AttributeInfos)
	{
		BroadCastAttributeInfoChanged(Info);
	}

	OnPlayerAttributePointsChanged.Broadcast(GetAuraPlayerState()->GetAttributePoints());
	OnPlayerSpellPointsChanged.Broadcast(GetAuraPlayerState()->GetSpellPoints());
}


void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	check(AttributeInfo);

	// TODO! Potential undefined behaviour! Maybe! It passes the ref., which I don't think is any good.
	// Should RETHINK THIS!
	for (FAuraAttributeInfo& Info : AttributeInfo->AttributeInfos)
	{
		GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(
			Info.Attribute).AddLambda(
			[this, &Info](const FOnAttributeChangeData& Data)
			{
				BroadCastAttributeInfoChanged(Info);
			});
	}

	GetAuraPlayerState()->OnAttributePointsChanged.AddLambda(
		[this](int32 NewAttributePoints)
		{
			OnPlayerAttributePointsChanged.Broadcast(NewAttributePoints);
		});
	GetAuraPlayerState()->OnSpellPointsChanged.AddLambda(
	[this](int32 NewSpellPoints)
	{
		OnPlayerSpellPointsChanged.Broadcast(NewSpellPoints);
	});
}


void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	GetAuraAbilitySystemComponent()->UpgradeAttribute(AttributeTag);
}


void UAttributeMenuWidgetController::BroadCastAttributeInfoChanged(FAuraAttributeInfo& Info) const
{
	Info.AttributeValue = Info.Attribute.GetNumericValue(GetAttributeSet());
	OnAttributeInfoChanged.Broadcast(Info);
}
