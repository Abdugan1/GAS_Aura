// Copyright Abdu Inc.


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include <rapidjson/schema.h>

#include "AbilitySystemBlueprintLibrary.h"
#include "AssetDefinitionAssetInfo.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Interaction/PlayerInterface.h"


void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientOnEffectAppliedToSelf);
}


void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	check(GetAvatarActor()->HasAuthority());
	
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1.f);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			/** Input Tag, with which you can activate the Ability */
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupInputTag);

			/** Setting the status to Equipped */
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);

			
			GiveAbility(AbilitySpec);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Expected AuraAbilities in AddCharacterAbilities! Skipping addition of the ability"))
		}
	}

	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}


void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	check(GetAvatarActor()->HasAuthority());

	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1.f);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}


void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	check(GetAvatarActor()->Implements<UPlayerInterface>());
	
	if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
	{
		ServerUpgradeAttribute(AttributeTag);
 	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
}

void UAuraAbilitySystemComponent::AbilityInputKeyReleased(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ScopedAbilityListLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);

			if (AbilitySpec.IsActive())
			{
				// NOTE: GetPrimaryInstance() in only valid on IntancedPerActor abilities.
				// If decided to use other instances, use GetAbilityInstances() and loop over it.
				// More on this: https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/40435524#questions/22659811
				UGameplayAbility* PrimaryIntance = AbilitySpec.GetPrimaryInstance();

				if (PrimaryIntance)
				{
					InvokeReplicatedEvent(
						EAbilityGenericReplicatedEvent::InputReleased,
						AbilitySpec.Handle,
						PrimaryIntance->GetCurrentActivationInfo().GetActivationPredictionKey()
						);
				}
			}

		}
	}
}


void UAuraAbilitySystemComponent::AbilityInputKeyHeld(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock ScopedAbilityListLock(*this);	
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputKeyPressed(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}
	
	FScopedAbilityListLock ScopedAbilityListLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				// NOTE: GetPrimaryInstance() in only valid on IntancedPerActor abilities.
				// If decided to use other instances, use GetAbilityInstances() and loop over it.
				// More on this: https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/40435524#questions/22659811
				UGameplayAbility* PrimaryIntance = AbilitySpec.GetPrimaryInstance();

				if (PrimaryIntance)
				{
					InvokeReplicatedEvent(
						EAbilityGenericReplicatedEvent::InputPressed,
						AbilitySpec.Handle,
						PrimaryIntance->GetCurrentActivationInfo().GetActivationPredictionKey()
						);
				}
			}
		}
	}
	
}



void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActivateScopeLock{*this};
	for (const auto& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}


void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid())
		{
			continue;
		}
		
		if (Level < Info.LevelUpRequirement)
		{
			continue;
		}
		
		// We only care about Abilities that Aura doesn't have yet.
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec(Info.AbilityClass, 1);
			// We reached the required level, and we don't have the ability, so we make it Eligible
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);


			/**
			 * Sidenote: I think we can also remove MarkAbilitySpecDirty()
			 * as it is already being called for the spec inside OnGiveAbility(), after successfully giving the ability.
			 */
			// MarkAbilitySpecDirty(AbilitySpec); 		// Forcing to Replicate
	
			/**
			 * Issue using ClientUpdateAbilityStatus here:
			 * - AbilityScopeLockCount is not null here after GiveAbility(AbilitySpec) so when wanting to enable Spell Buttons,
			 *   GetActivatableAbilities doesn't contain the AbilitySpec!
			 * Solution: https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/39869210#questions/22525277
			*/
			// ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible);
		}
	}
}


FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActivateScopeLock{*this};
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (const FGameplayTag& Tag : AbilitySpec.Ability->GetAssetTags())
		{
			// TODO: For some reason, Stephen used MatchesTag, not the Exact version
			if (Tag.MatchesTagExact(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}

	return nullptr;
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
	FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		// Ability is NOT Locked
		UAuraGameplayAbility *AuraAbility = CastChecked<UAuraGameplayAbility>(Spec->Ability);
		OutDescription = AuraAbility->GetDescription(Spec->Level);
		OutNextLevelDescription = AuraAbility->GetNextLevelDescription(Spec->Level + 1);
		return true;
	}
	// Ability is Locked
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		// If nothing is selected, don't do anything
		OutDescription = "";
	}
	else if (GetAvatarActor()->HasAuthority())
	{
		/**
		 * ERROR: This DOES NOT WORK on CLIENTS because it fetches AbilityInfo from Game Mode, which exist only on the SERVER!
		 * Fix available on my Telegram channel
		 */
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoFromTag(AbilityTag).LevelUpRequirement);
	}
	else
	{
		OutDescription = "ERROR: Doesn't work on CLIENTS!";
	}


	
	OutNextLevelDescription = FString();
	return false;
}


FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName{"Abilities"})))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag{};
}


FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName{"Input"})))
		{
			return Tag;
		}
	}
	return FGameplayTag{};
}


FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName{"Abilities.Status"})))
		{
			return Tag;
		}
	}
	return FGameplayTag{};
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& SlotTag)
{
	FScopedAbilityListLock Lock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, SlotTag))
		{
			return false;
		}
	}
	return true;
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& SlotTag)
{
	return AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(SlotTag);
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& AbilitySpec)
{
	return AbilitySpec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName{"Input"}));
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& AbilitySpec) const
{
	const UAbilityInfo* Info = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(AbilitySpec);
	const FAuraAbilityInfo& AbilityInfo = Info->FindAbilityInfoFromTag(AbilityTag);
	const FGameplayTag AbilityType = AbilityInfo.AbilityTypeTag;
	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock Lock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& SlotTag)
{
	RemoveAbilityFromItsSlot(*AbilitySpec); // I think this is redundant, since we already did that.
	AbilitySpec->GetDynamicSpecSourceTags().AddTag(SlotTag);
}

void UAuraAbilitySystemComponent::ServerEquipAbilityToSlot_Implementation(const FGameplayTag& AbilityTag,
                                                                          const FGameplayTag& ToSlot)
{
	FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag);
	check(AbilitySpec);

	const FGameplayTag PreviousSlot = GetInputTagFromSpec(*AbilitySpec);
	const FGameplayTag StatusTag = GetStatusFromSpec(*AbilitySpec);

	// Fool-proofing - only Equipped or Unlocked abilities can be equipped
	check(StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped)
		|| StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Unlocked));

	if (!SlotIsEmpty(ToSlot)) // There's an ability in this slot already. Deactivate and clear its slot.
	{
		FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(ToSlot);
		if (SpecWithSlot) // Valid Slot
		{
			// Check if the ability we're trying to equip to the slot is the same ability that's already there.
			// That is, if the "replacy" is equal to "replacer"
			// If so, well, nothing needed to be replaced. Just return.
			if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
			{
				// This is NEEDED! Because the UI has an animated rectange showing the user where to click and equip.
				// With this, the rectangle will disappear. 
				ClientEquipAbilityToSlot(AbilityTag, StatusTag, ToSlot, PreviousSlot);
				return;
			}

			// If it's passive ability, deactivate it.
			if (IsPassiveAbility(*SpecWithSlot))
			{
				DeactivatePassiveAbilityDelegate.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
			}

			RemoveAbilityFromItsSlot(*SpecWithSlot);
		}
	}


	if (!AbilityHasAnySlot(*AbilitySpec)) // Doesn't yet have any Slot (it's not active), so ->...
	{
		// ...-> if it's passive, activate it ONLY WHEN IT WAS EQUIPPED FOR THE FIRST TIME!
		if (IsPassiveAbility(*AbilitySpec))
		{
			TryActivateAbility(AbilitySpec->Handle);
		}
	}

	AssignSlotToAbility(AbilitySpec, ToSlot);
	
	// If it wasn't equipped before, change its status to Equipped
	if (StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Unlocked))
	{
		AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(FAuraGameplayTags::Get().Abilities_Status_Unlocked);
		AbilitySpec->GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
	}

	MarkAbilitySpecDirty(*AbilitySpec);
	
	ClientEquipAbilityToSlot(AbilityTag, StatusTag, ToSlot, PreviousSlot);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(Slot))
		{
			RemoveAbilityFromItsSlot(Spec);
		}
	}
}

void UAuraAbilitySystemComponent::RemoveAbilityFromItsSlot(FGameplayAbilitySpec& AbilitySpec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(AbilitySpec);
	AbilitySpec.GetDynamicSpecSourceTags().RemoveTag(Slot);
}

void UAuraAbilitySystemComponent::ClientEquipAbilityToSlot_Implementation(const FGameplayTag& AbilityTag,
                                                                          const FGameplayTag& AbilityStatusTag, const FGameplayTag& ToSlot, const FGameplayTag& PreviousSlot)
{
	AbilityEquippedDelegate.Broadcast(AbilityTag, AbilityStatusTag, ToSlot, PreviousSlot);
}


void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag);
	FGameplayTag AbilityStatus = GetStatusFromSpec(*AbilitySpec);
	
	/* Fool-proofing */
	check(AbilitySpec != nullptr); // The Ability MUST be in the ASC's GetActivatableAbilities
	check(GetAvatarActor()->Implements<UPlayerInterface>()); 
	check(IPlayerInterface::Execute_GetSpellPoints(GetAvatarActor()) > 0); // Cannot do anything if not enough SpellPoints
	check(AbilityStatus.MatchesTag(FAuraGameplayTags::Get().Abilities_Status_Locked) == false); // If the Ability is Locked, not possible to do anything

	IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
	
	if (AbilityStatus == FAuraGameplayTags::Get().Abilities_Status_Eligible)
	{
		// If I didn't remove the initial Eligible status, the Ability would have two Status GameplayTags...
		AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
		AbilitySpec->GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Unlocked);

		AbilityStatus = FAuraGameplayTags::Get().Abilities_Status_Unlocked;
	}
	else if (AbilityStatus == FAuraGameplayTags::Get().Abilities_Status_Unlocked
		|| AbilityStatus == FAuraGameplayTags::Get().Abilities_Status_Equipped)
	{
		// It's a Replicated var
		AbilitySpec->Level += 1;
	}

	ClientUpdateAbilityStatus(AbilityTag, AbilityStatus);

	// Force Replication
	MarkAbilitySpecDirty(*AbilitySpec);
}


void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	/** Since this is not a replicated variable, it's still false on the client. */
	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UAuraAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	const bool bIsLocallyControlled = AbilityActorInfo->IsLocallyControlled();
	if (!bIsLocallyControlled)
	{
		return;
	}

	const FGameplayTag StatusTag = GetStatusFromSpec(AbilitySpec);
	if (StatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Eligible))
	{
		AbilityStatusChangedDelegate.Broadcast(GetAbilityTagFromSpec(AbilitySpec), StatusTag);
	}
}


void UAuraAbilitySystemComponent::ClientOnEffectAppliedToSelf_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                             const FGameplayEffectSpec& EffectSpec,
                                                                             FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer AssetTags;
	EffectSpec.GetAllAssetTags(AssetTags);

	EffectAppliedToSelf.Broadcast(AssetTags);
}


void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& StatusTag)
{
	AbilityStatusChangedDelegate.Broadcast(AbilityTag, StatusTag);
}