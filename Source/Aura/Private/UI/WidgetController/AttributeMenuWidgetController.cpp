// Copyright Abdu Inc.


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"


void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	check(AttributeInfo);

	// TODO! Why can I not use values? I guess I can here. But in the BindCallbacks...
	for (FAuraAttributeInfo& Info : AttributeInfo->AttributeInfos)
	{
		BroadCastAttributeInfoChanged(Info);
	}
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
}


void UAttributeMenuWidgetController::BroadCastAttributeInfoChanged(FAuraAttributeInfo& Info) const
{
	UE_LOG(LogTemp, Display, TEXT("Broadcasting... %f"), Info.Attribute.GetNumericValue(AttributeSet));
	Info.AttributeValue = Info.Attribute.GetNumericValue(AttributeSet);
	OnAttributeInfoChanged.Broadcast(Info);
}
