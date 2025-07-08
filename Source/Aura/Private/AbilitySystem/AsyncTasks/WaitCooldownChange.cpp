// Copyright Abdu Inc.


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* ASC,
	const FGameplayTag& InCooldownTag)
{
	auto WaitCooldownChange = NewObject<UWaitCooldownChange>();

	WaitCooldownChange->ASC = ASC;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(ASC) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// To know when a cooldown has ended (Cooldown Tag has been removed)
	ASC->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved)
	.AddUObject(WaitCooldownChange, &UWaitCooldownChange::CooldownTagChanged);

	// To know when a cooldown has been applied
	/**
	 * VERY IMPORTANT NOTE:
	 * This is called ONCE on the SERVER,
	 * BUT, callded TWICE on CLIENTS!
	 * For the PREDICTED GE, and the server's REPLICATED ONE.
	 * Check out Diego's answer:
	 * https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/40847062#questions/20624870
	 */
	ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);
	
	return WaitCooldownChange;
}


void UWaitCooldownChange::EndTask()
{
	check(ASC);
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	SetReadyToDestroy();
	MarkAsGarbage();
}


void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}


void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC,
	const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveGameplayEffect)
{
	/**
	 * VERY IMPORTANT NOTE:
	 * This is called ONCE on the SERVER,
	 * BUT, callded TWICE on CLIENTS!
	 * For the PREDICTED GE, and the server's REPLICATED ONE.
	 * Check out Diego's answer:
	 * https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/40847062#questions/20624870
	 */

	/**
	 * Diego decided to remove this because he was having some issues with the Electrocute ability.
	 * I think it works for me no problem.
	 */
	const bool bIsReplicatedEffect = !SpecApplied.GetContext().GetAbilityInstance_NotReplicated();
	if (bIsReplicatedEffect)
	{
		return;
	}

	/**
	 * This is Chelsey's solution. I don't know how he uses it though.
	 */
	// const bool bHasNotNonSnapAttrs = !SpecApplied.CapturedRelevantAttributes.HasNonSnapshottedAttributes();
	
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		const auto TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			// auto TimeRemaining = *(std::max_element(TimesRemaining.begin(), TimesRemaining.end()));
			auto TimeRemaining = TimesRemaining[0];
			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
