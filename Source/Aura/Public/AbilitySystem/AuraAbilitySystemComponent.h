// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

// TODO: Must rename the name of it.
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /* AssetTags */)
DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);
DECLARE_MULTICAST_DELEGATE_TwoParams(FAbilityStatusChanged,  const FGameplayTag& /* AbilityTag */, const FGameplayTag& /* StatusTag */);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /* AbilityStatusTag */, const FGameplayTag& /* ToSlot */, const FGameplayTag& /* PreviousSlot */);
DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /** AbilityTag */)

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

	/** Broadcasts when Initial Abilities were given */
	FAbilitiesGiven AbilitiesGivenDelegate;

	/** Broadcasts whenever Ability Status changes */
	FAbilityStatusChanged AbilityStatusChangedDelegate; 

	/** */
	FAbilityEquipped AbilityEquippedDelegate;

	/** */
	FDeactivatePassiveAbility DeactivatePassiveAbilityDelegate;
	
	/** Sometimes, it's hard to know the timelapse, so this is set along with AbilitiesGivenDelegate broadcast */
	bool bStartupAbilitiesGiven = false;

	/**
	 * Give startup abilities. Must be called server only
	 * NOTE: Abilities given must be AuraGameplayAbility only!
	 * Automatically adds AuraGameplayAbility::StartupInputTag to DynamicAbilityTags
	 */
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);

	/**
	 * Increments the Attribute's value. In this game, these are Primary Attributes
	 */
	void UpgradeAttribute(const FGameplayTag& AttributeTag); 

	/** Calls AbilitySpecInputReleased. Does NOT cancel the ability. */
	void AbilityInputKeyReleased(FGameplayTag InputTag);

	/** Calls AbilitySpecInputPressed. TRYies to enable the ability. Does NOT enable if already enabled */
	void AbilityInputKeyHeld(FGameplayTag InputTag);

	void AbilityInputKeyPressed(FGameplayTag InputTag);

	/** Loop through GetActivatableAbilities and execute the delegate's functor */
	void ForEachAbility(const FForEachAbility& Delegate);

	/** Updates Abilities' Statuses based on Level provided. For example, making an Ability Eligible */
	void UpdateAbilityStatuses(int32 Level);

	/** Returns Spec if Ability exists on ASC, otherwise return nullptr */
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);

	/**
	 * Fills out both OutDescription and OutNextLevelDescription. Return FALSE if the Ability is locked
	 * NOTE: I'm using AbilityLevel because I use this function where AbilitySpec->Level is NOT REPLICATED!
	 */ 
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);
	
	/** Return GameplayTag -> Abilities.* from AbilityTags; Returns an empty GameplayTag if failed to find any */
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	/** Return GameplayTag -> InputTag.* from DynamicAbilityTags; Returns an empty GameplayTag if failed to find any */
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	/** Returns GameplayTag -> Abilities.Status.* from DynamicAbilityTags. Returns an empty GameplayTag if failed to find any */
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	UFUNCTION(Server, Reliable)
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag);

	/** Equips an Ability to a Slot. Sends back to client what Ability was assigned to what Slot */
	UFUNCTION(Server, Reliable)
	void ServerEquipAbilityToSlot(const FGameplayTag& AbilityTag, const FGameplayTag& ToSlot);

	/** Broadcasts AbilityEquippedDelegate */
	UFUNCTION(Client, Reliable)
	void ClientEquipAbilityToSlot(const FGameplayTag& AbilityTag, const FGameplayTag& AbilityStatusTag, const FGameplayTag& ToSlot, const FGameplayTag& PreviousSlot); 
	
protected:
	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);
	
	/**
	 * This gets called whenever an effect is applied to self.
	 * NOTE: To get this callback to work, users MUST call AbilityActorInfoSet. See the definition for more.
	 */
	 UFUNCTION(Client, Reliable)
	void ClientOnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);

	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag);

	/**
	 * Since AddCharacterAbilities is called on the SERVER, clients cannot init their overlay properly.
	 * So we are broadcasting AbilitiesGivenDelegate when all the abilities were given
	 */
	virtual void OnRep_ActivateAbilities() override;
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;

private:
	/** Removes all the abilities assigned to a Slot. Basically, removes only one as it's only possible for a slot to have only one ability */
	void ClearAbilitiesOfSlot(const FGameplayTag& Slot);
	void RemoveAbilityFromItsSlot(FGameplayAbilitySpec& AbilitySpec);
};