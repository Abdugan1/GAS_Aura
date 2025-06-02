// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

// TODO: Must rename the name of it.
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /* AssetTags */)
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitiesGiven, UAuraAbilitySystemComponent* /* AuraASC */);

/**
 * Ugly name. But all it does is execute the function this delegate was bound to. Like a fancy callback passed to for_each
* 	FForEachAbility Delegate;
	Delegate.BindLambda([this, AuraASC](const FGameplayAbilitySpec& AbilitySpec)
	{
		// Do something here...
	});

	This will be called for every item
 */
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);

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
	FEffectAssetTags EffectAppliedToSelf;
	
	FAbilitiesGiven AbilitiesGivenDelegate;

	bool bStartupAbilitiesGiven = false;

	/**
	 * Give startup abilities. Must be called server only
	 * NOTE: Abilities given must be AuraGameplayAbility only!
	 * Automatically adds AuraGameplayAbility::StartupInputTag to DynamicAbilityTags
	 */
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);

	// void AbilityInputTagPressed(FGameplayTag InputTag);

	/** Calls AbilitySpecInputReleased. Does NOT cancel the ability. */
	void AbilityInputKeyReleased(FGameplayTag InputTag);

	/** Calls AbilitySpecInputPressed. TRYies to enable the ability. Does NOT enable if already enabled */
	void AbilityInputKeyHeld(FGameplayTag InputTag);

	/** Loop through GetActivatableAbilities and execute the delegate's functor */
	void ForEachAbility(const FForEachAbility& Delegate);

	/** Return GameplayTag -> Abilities.* from AbilityTags; Returns an emtpy GameplayTag if failed to find any */
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	/** Return GameplayTag -> InputTag.* from DynamicAbilityTags; Returns an emtpy GameplayTag if failed to find any */
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	
protected:
	/**
	 * This gets called whenever an effect is applied to self.
	 * NOTE: To get this callback to work, users MUST call AbilityActorInfoSet. See the definition for more.
	 */
	 UFUNCTION(Client, Reliable)
	void ClientOnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);

	/**
	 * Since AddCharacterAbilities is called on the server, clients cannot init their overlay properly.
	 * So we are broadcasting AbilitiesGivenDelegate when all the abilities were given
	 */
	virtual void OnRep_ActivateAbilities() override;
};
