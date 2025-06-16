// Copyright Abdu Inc.


#include "UI/WidgetController/AuraWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"


void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams)
{
	PlayerController = WidgetControllerParams.PlayerController;
	PlayerState = WidgetControllerParams.PlayerState;
	AbilitySystemComponent = WidgetControllerParams.AbilitySystemComponent;
	AttributeSet = WidgetControllerParams.AttributeSet;
}


void UAuraWidgetController::BroadcastInitialValues()
{
}


void UAuraWidgetController::BindCallbacksToDependencies()
{
}

void UAuraWidgetController::BroadcastInitialAbilitiesInfo()
{
	UAuraAbilitySystemComponent *AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	if (!AuraASC->bStartupAbilitiesGiven)
	{
		UE_LOG(LogAura, Error, TEXT("BroadcastInitialAbilitiesInfo called but their were not given!"));
		return;
	}

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, AuraASC](const FGameplayAbilitySpec& AbilitySpec)
	{
		FGameplayTag AbilityTag = AuraASC->GetAbilityTagFromSpec(AbilitySpec);
		FAuraAbilityInfo AbilityInfo = AbilitiesInfo->FindAbilityInfoFromTag(AbilityTag);
		FGameplayTag StatusTag = AuraASC->GetStatusFromSpec(AbilitySpec);
		
		AbilityInfo.InputTag = AuraASC->GetInputTagFromSpec(AbilitySpec);
		AbilityInfo.StatusTag = StatusTag;
		AbilityInfoDelegate.Broadcast(AbilityInfo);
	});
	AuraASC->ForEachAbility(BroadcastDelegate);
}
