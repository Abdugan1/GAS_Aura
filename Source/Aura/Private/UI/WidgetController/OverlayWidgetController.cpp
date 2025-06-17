// Copyright Abdu Inc.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/Data/LevelUpInfo.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetAuraAttributeSet()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAttributeSet()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAttributeSet()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAttributeSet()->GetMaxMana());
}


void UOverlayWidgetController::BindCallbacksToDependencies()
{
	BindAttributeChangesToDelegates();
	
	if (GetAuraAbilitySystemComponent()->bStartupAbilitiesGiven)
	{
		BroadcastInitialAbilitiesInfo();
	}
	else
	{
		GetAuraAbilitySystemComponent()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastInitialAbilitiesInfo);
	}

	BindToEffectApplicationToShowMessageOnScreen();

	GetAuraPlayerState()->OnXPChanged.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPlayerState()->OnLevelChanged.AddLambda(
		[this](int32 NewLevel)
	{
		OnPlayerLevelChanged.Broadcast(NewLevel);
	});
}


void UOverlayWidgetController::BindAttributeChangesToDelegates()
{
	/** Health **/
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(
		GetAuraAttributeSet()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			});
	/** MaxHealth **/
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(
	GetAuraAttributeSet()->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
	/** Mana **/
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(
	GetAuraAttributeSet()->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			});
	/** MaxMana **/
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(
	GetAuraAttributeSet()->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			});
}


void UOverlayWidgetController::BindToEffectApplicationToShowMessageOnScreen()
{
	// Broadcast the row with the info about the effect applied
	GetAuraAbilitySystemComponent()->EffectAppliedToSelf.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			for (const FGameplayTag& Tag : AssetTags)
			{
				// For example, say that Tag = Message.HealthPotion
				// "Message.HealthPotion".MatchesTag("Message") -> true
				// "Message".MatchesTag("Message.HealthPotion") -> false
				static FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				if (Tag.MatchesTag(MessageTag))
				{
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row);
				}
			}
		}
		);
}


void UOverlayWidgetController::OnXPChanged(int32 NewXP) const
{
	// TODO: Can't use the function version, GetAuraPlayerState since this we are inside a const function.
	const AAuraPlayerState* AuraPS = CastChecked<AAuraPlayerState>(GetPlayerState());
	const TObjectPtr<ULevelUpInfo> LevelUpInfo = AuraPS->LevelUpInfo;

	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInfos.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInfos[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInfos[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChanged.Broadcast(XPBarPercent);
	}
}