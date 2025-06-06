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


	AAuraPlayerState* AuraPS = CastChecked<AAuraPlayerState>(PlayerState);
	OnPlayerAttributePointsChanged.Broadcast(AuraPS->GetAttributePoints());
	OnPlayerSpellPointsChanged.Broadcast(AuraPS->GetSpellPoints());
}


void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	check(AttributeInfo);

	// TODO! Potential undefined behaviour! Maybe! It passes the ref., which I don't think is any good.
	// Should RETHINK THIS!
	for (FAuraAttributeInfo& Info : AttributeInfo->AttributeInfos)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			Info.Attribute).AddLambda(
			[this, &Info](const FOnAttributeChangeData& Data)
			{
				BroadCastAttributeInfoChanged(Info);
			});
	}

	AAuraPlayerState* AuraPs = CastChecked<AAuraPlayerState>(PlayerState);
	AuraPs->OnAttributePointsChanged.AddLambda(
		[this](int32 NewAttributePoints)
		{
			OnPlayerAttributePointsChanged.Broadcast(NewAttributePoints);
		});
	AuraPs->OnSpellPointsChanged.AddLambda(
	[this](int32 NewSpellPoints)
	{
		OnPlayerSpellPointsChanged.Broadcast(NewSpellPoints);
	});
}


void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(AttributeTag);
}


void UAttributeMenuWidgetController::BroadCastAttributeInfoChanged(FAuraAttributeInfo& Info) const
{
	UE_LOG(LogTemp, Display, TEXT("Broadcasting... %f"), Info.Attribute.GetNumericValue(AttributeSet));
	Info.AttributeValue = Info.Attribute.GetNumericValue(AttributeSet);
	OnAttributeInfoChanged.Broadcast(Info);
}
