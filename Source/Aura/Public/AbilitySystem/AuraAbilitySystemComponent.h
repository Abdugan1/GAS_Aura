// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

// TODO: Must rename the name of it.
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /* AssetTags */)

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	/**
	 * Users MUST call this if they need to broadcast effects applied.
	 * NOTE: Called after InitAbilityActorInfo.
	 */
	void AbilityActorInfoSet();

	/*
	 * Broadcasts the effect's AssetTags whenever it's applied to self.
	 */
	FEffectAssetTags EffectAssetTags;

	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

	// void AbilityInputTagPressed(FGameplayTag InputTag);

	/** Calls AbilitySpecInputReleased. Does NOT cancel the ability. */
	void AbilityInputKeyReleased(FGameplayTag InputTag);

	/** Calls AbilitySpecInputPressed. TRYies to enable the ability. Does NOT enable if already enabled */
	void AbilityInputKeyHeld(FGameplayTag InputTag);
	
protected:
	/**
	 * This gets called whenever an effect is applied to self.
	 * NOTE: To get this callback to work, users MUST call AbilityActorInfoSet. See the definition for more.
	 */
	 UFUNCTION(Client, Reliable)
	void ClientOnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
};
