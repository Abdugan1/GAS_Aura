// Copyright Abdu Inc.


#include "AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Aura.h"
#include "Aura/AuraLogChannels.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Player/AuraPlayerController.h"


UAuraAttributeSet::UAuraAttributeSet()
{
}


void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	// It seems it's changing the CurrentValue. So when you pick up several health potions, your Health actually exceeds the HealthMax.
	// So when you get hurt, health does not change until the BaseValue is lower than the MaxValue.
	// There's another function named 'PreAttributeBaseValue'. The same logic is applied there, but it clamps both values.

	// Future me. The comment above is not accurate. Check the docs to gain true explanation.
	// For now, I know that this does clamp, but not what you expect. It ONLY clamps the final value.
	// Something out there still has a different value. If you overflow your health to 150,
	// and your MaxHealth is 100, then it clamps the final value to 100, but somewhere there, there's still
	// calculations going on to 150!
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
}


void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	FEffectProperties EffectProperties;
	SetEffectProperties(Data, EffectProperties);

	// Skip everything if Target is dead.
	if (EffectProperties.TargetCharacter->Implements<UCombatInterface>())
	{
		if (ICombatInterface::Execute_IsDead(EffectProperties.TargetCharacter))
		{
			return;
		}
	}
	
	// This helps to prevent overflow. Doesn't understand how though.
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
	if (Data.EvaluatedData.Attribute == GetIncomingXpAttribute())
	{
		HandleIncomingXp(EffectProperties);
	}
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(EffectProperties);
	}
}

void UAuraAttributeSet::HandleIncomingXp(const FEffectProperties& EffectProperties)
{
	const float LocalIncomingXp = GetIncomingXp();
	SetIncomingXp(0.f);

	/** Source Character is the owner, since GA_ListenForEvents applies GE_EventBasedEffect, add to IncomingXP */
	checkf(EffectProperties.SourceCharacter->Implements<UPlayerInterface>(), TEXT("Source Character doesn't implement UPlayerInterface. Fix that."));

	const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(EffectProperties.SourceCharacter);
	const int32 CurrentXP = IPlayerInterface::Execute_GetXP(EffectProperties.SourceCharacter); 

	const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXp(EffectProperties.SourceCharacter, CurrentXP + LocalIncomingXp);

	const int32 NumLevelUps = NewLevel - CurrentLevel;
	if (NumLevelUps > 0)
	{
		int32 AttributePointsReward = 0;
		int32 SpellPointsReward = 0;

		for (int32 i = 0; i < NumLevelUps; i++)
		{
			AttributePointsReward += IPlayerInterface::Execute_GetAttributePointsReward(EffectProperties.SourceCharacter, CurrentLevel + i);
			SpellPointsReward += IPlayerInterface::Execute_GetSpellPointsReward(EffectProperties.SourceCharacter, CurrentLevel + i);
		}

		IPlayerInterface::Execute_AddToPlayerLevel(EffectProperties.SourceCharacter, NumLevelUps);
		IPlayerInterface::Execute_AddToAttributePoints(EffectProperties.SourceCharacter, AttributePointsReward);
		IPlayerInterface::Execute_AddToSpellPoints(EffectProperties.SourceCharacter, SpellPointsReward);

		bTopOffHealth = true;
		bTopOffMana = true;
			
		IPlayerInterface::Execute_LevelUp(EffectProperties.SourceCharacter);
	}
		
	IPlayerInterface::Execute_AddToXp(EffectProperties.SourceCharacter, LocalIncomingXp);
}

void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& EffectProperties)
{
	// Damage is dealt by a meta attribute called IncomingDamage
	// Here, we make the target die if the damage was fatal,
	// else, we play hit react. Hit React is a common ability for all characters.
	// We activate it by the Effect.HitReact tag. Also, we show a floating text of damage dealt.
	// The text is only visible to the source, that is, if a client dealt the damage, only that client
	// will see the floating text. Even the server will not see it.
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0);
	if (LocalIncomingDamage > 0.f)
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

		const bool bFatal = NewHealth <= 0.f;

		if (bFatal)
		{
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(EffectProperties.TargetAvatarActor))
			{
				CombatInterface->Die(UAuraAbilitySystemLibrary::GetDeathImpulse(EffectProperties.EffectContextHandle));
			}
			SendXPEvent(EffectProperties);
		}
		else
		{
			// Is TargetCharacter NOT being shocked? If so, HitReact, otherwise, ShockLoop in ABP
			if (EffectProperties.TargetCharacter->Implements<UCombatInterface>()
				&& !ICombatInterface::Execute_IsBeingShocked(EffectProperties.TargetCharacter))
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::Get().Abilities_HitReact);
				EffectProperties.TargetAbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
			}
			
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(EffectProperties.TargetAvatarActor))
			{
				const auto Knockback = UAuraAbilitySystemLibrary::GetKnockbackImpulse(EffectProperties.EffectContextHandle);
				CombatInterface->ApplyKnockback(UAuraAbilitySystemLibrary::GetKnockbackImpulse(EffectProperties.EffectContextHandle));
			}
		}

		const bool bBlockedHit = UAuraAbilitySystemLibrary::IsBlockedHit(EffectProperties.EffectContextHandle);
		const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(EffectProperties.EffectContextHandle);
		ShowFloatingText(EffectProperties, LocalIncomingDamage, bBlockedHit, bCriticalHit);

		if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(EffectProperties.EffectContextHandle))
		{
			Debuff(EffectProperties);
		}
	}
}

void UAuraAttributeSet::Debuff(const FEffectProperties& EffectProperties)
{
	/**
	 * There's a post questioning using Dynamic GEs.
	 * Dynamic GEs are for educational purposes, Stephen says.
	 * https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/40456796#questions/21215166
	 */

	/**
	 * Also, there's a quite complicated post about taking resistances into account.
	 * https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/40456796#questions/21348082
	 */

	/** Dynamic GEs have limitations, like not replicating, or at least not executing on clients */
	
	FGameplayEffectContextHandle DebuffEffectContextHandle = EffectProperties.SourceAbilitySystemComponent->MakeEffectContext();
	DebuffEffectContextHandle.AddSourceObject(EffectProperties.SourceAvatarActor);

	const FGameplayTag DamageTypeTag =  UAuraAbilitySystemLibrary::GetDamageTypeTag(EffectProperties.EffectContextHandle);
	const float DebuffDamage = UAuraAbilitySystemLibrary::GetDebuffDamage(EffectProperties.EffectContextHandle);
	const float DebuffDuration = UAuraAbilitySystemLibrary::GetDebuffDuration(EffectProperties.EffectContextHandle);
	const float DebuffFrequency = UAuraAbilitySystemLibrary::GetDebuffFrequency(EffectProperties.EffectContextHandle);
	
	const FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DamageTypeTag.ToString());

	UGameplayEffect* DebuffEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));
	DebuffEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DebuffEffect->Period = DebuffFrequency;
	DebuffEffect->DurationMagnitude = FScalableFloat(DebuffDuration);

	// GrantingTags
	
	/** This is DEPRICATED */
	// DebuffEffect->InheritableOwnedTagsContainer.AddTag(FAuraGameplayTags::Get().DamageTypesToDebuffs[DamageTypeTag]);
	
	/**
	 * This is the same thing of the DEPRICATED code.
	 * Need to use #include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h" 
	 */
	FInheritedTagContainer TagContainer = FInheritedTagContainer();
	UTargetTagsGameplayEffectComponent& Component = DebuffEffect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();

	/**
	 * NOTE: I'm not sure why we're setting both Added and CombinedTags
	 * Does this mean that these tags are not replicated to clients?
	 * I'm setting Player_Block_* Tags in a RepNotify function inside AuraCharacter whenever bIsStunned is replicated. 
	 */
	const FGameplayTag DebuffTag = FAuraGameplayTags::Get().DamageTypesToDebuffs[DamageTypeTag]; 
	TagContainer.Added.AddTag(DebuffTag);
	TagContainer.CombinedTags.AddTag(DebuffTag);
	
	Component.SetAndApplyTargetTagChanges(TagContainer);

	if (DebuffTag.MatchesTagExact(FAuraGameplayTags::Get().Debuff_Stun))
	{
		/** NOTE: I'm not sure why we're setting both Added and CombinedTags */
		TagContainer.Added.AddTag(FAuraGameplayTags::Get().Player_Block_CursorTrace);
		TagContainer.Added.AddTag(FAuraGameplayTags::Get().Player_Block_InputHeld);
		TagContainer.Added.AddTag(FAuraGameplayTags::Get().Player_Block_InputPressed);
		TagContainer.Added.AddTag(FAuraGameplayTags::Get().Player_Block_InputReleased);
		
		TagContainer.CombinedTags.AddTag(FAuraGameplayTags::Get().Player_Block_CursorTrace);
		TagContainer.CombinedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputHeld);
		TagContainer.CombinedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputPressed);
		TagContainer.CombinedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputReleased);
	}

	// End of GrantingTags

	DebuffEffect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	DebuffEffect->StackLimitCount = 1;

	// Why can I not do this? Or can I?
	// FGameplayModifierInfo ModifierInfo;
	// ... do the stuff
	// DebuffEffect->Modifiers.Add(FGameplayModifierInfo()); 
	const int32 Index = DebuffEffect->Modifiers.Num();
    DebuffEffect->Modifiers.Add(FGameplayModifierInfo());
    FGameplayModifierInfo& ModifierInfo = DebuffEffect->Modifiers[Index];
	
    ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);
    ModifierInfo.ModifierOp = EGameplayModOp::Additive;
    ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();

	if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(DebuffEffect, DebuffEffectContextHandle, 1.f))
	{
		// For some reason, Stephen is not using the function from our Library.
		// Maybe he forgot about that. Anyway, NOTE: I don't know if it's legal to use Ar << GameplayTag,
		// but I did it that way. Stephen did it using the method used with the var HitResult.
		
		// FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
		// TSharedPtr<FGameplayTag> DebuffDamageType = MakeShareable(new FGameplayTag(DamageType));
		// AuraContext->SetDamageType(DebuffDamageType);
		UAuraAbilitySystemLibrary::SetDamageTypeTag(DebuffEffectContextHandle, DamageTypeTag);

		EffectProperties.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}
}

void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	/**
	 * After we level up, our Max Health and Max Mana also gets their value updated, that is, they're bigger than before.
	 * So, changing it after, that is, PostAttributeChange, we correctly set Health and Mana value to their maximum.
	 */
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		SetHealth(GetMaxHealth());
		bTopOffHealth = false;
	}
	if (Attribute == GetMaxManaAttribute() && bTopOffMana)
	{
		SetMana(GetMaxMana());
		bTopOffMana = false;
	}
}


void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data,
                                            FEffectProperties& EffectProperties) const
{
	// Source = causer of the effect, Target = target of the effect (owner of this AS)

	// Source operations
	EffectProperties.EffectContextHandle = Data.EffectSpec.GetContext();
	EffectProperties.SourceAbilitySystemComponent = EffectProperties.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	
	if (IsValid(EffectProperties.SourceAbilitySystemComponent)
		&& EffectProperties.SourceAbilitySystemComponent->AbilityActorInfo.IsValid()
		&& EffectProperties.SourceAbilitySystemComponent->AbilityActorInfo->AvatarActor.IsValid())
	{
		// Same as getting it from AbilityActorInfo->AvatarActor(). GetAvatarActor() has a check in it.
		// We are checking in the 'if' statement if the data required is valid. Don't know why though...
		EffectProperties.SourceAvatarActor = EffectProperties.SourceAbilitySystemComponent->GetAvatarActor();
		EffectProperties.SourceController = EffectProperties.SourceAbilitySystemComponent->AbilityActorInfo.Get()->PlayerController.Get();

		// Try to obtain the PlayerController from the Avatar anyway.
		if (EffectProperties.SourceController == nullptr && EffectProperties.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(EffectProperties.SourceAvatarActor))
			{
				EffectProperties.SourceController = Pawn->GetController();
			}
		}

		if (EffectProperties.SourceController != nullptr)
		{
			// EffectProperties.SourceCharacter = EffectProperties.SourceController->GetCharacter(); // For some reason, this doesn't work for enemies
			EffectProperties.SourceCharacter = Cast<ACharacter>(EffectProperties.SourceController->GetPawn());
		}
	}

	// Target operations
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		EffectProperties.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		EffectProperties.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		EffectProperties.TargetCharacter = Cast<ACharacter>(EffectProperties.TargetAvatarActor);
		EffectProperties.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
			EffectProperties.TargetAvatarActor);
	}
}


void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& EffectProperties, float Damage, bool bBlockedHit,
	bool bCriticalHit) const
{
	if (EffectProperties.SourceCharacter != EffectProperties.TargetCharacter)
	{
		/** NOTE: This returns the server's player controller if the index is 0. */
		
		// if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(
		// 	UGameplayStatics::GetPlayerController(EffectProperties.SourceCharacter, 0)))
		// {
		// 	PC->ShowDamageNumber(Damage, EffectProperties.TargetCharacter);
		// }

		/** The correct way to do it? */

		// If Aura causing damage
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(EffectProperties.SourceCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, EffectProperties.TargetCharacter, bBlockedHit, bCriticalHit);
			return; // I need this because for some reason if I use "else if", it conflicts with this PC...
		}
		// If enemies causing damage
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(EffectProperties.TargetCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, EffectProperties.TargetCharacter, bBlockedHit, bCriticalHit);
		}
	}
}


void UAuraAttributeSet::SendXPEvent(const FEffectProperties& EffectProperties)
{
	if (EffectProperties.TargetCharacter->Implements<UCombatInterface>())
	{
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(EffectProperties.TargetCharacter);
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(EffectProperties.TargetCharacter);
	
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(EffectProperties.TargetCharacter, TargetClass, TargetLevel);

		const FGameplayTag IncomingXPTag = FAuraGameplayTags::Get().Attributes_Meta_IncomingXP;
		FGameplayEventData IncomingXPData;
		IncomingXPData.EventTag = IncomingXPTag;
		IncomingXPData.EventMagnitude = static_cast<float>(XPReward);
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(EffectProperties.SourceCharacter, IncomingXPTag, IncomingXPData);	
	}
}


void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistanceFire, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistanceLightning, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistanceArcane, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ResistancePhysical, COND_None, REPNOTIFY_Always);
	
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}


void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}


void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}


void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}


void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}


void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}


void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}


void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}


void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}


void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}


void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}


void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}


void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}


void UAuraAttributeSet::OnRep_ResistanceFire(const FGameplayAttributeData& OldResistanceFire) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistanceFire, OldResistanceFire);
}


void UAuraAttributeSet::OnRep_ResistanceLightning(const FGameplayAttributeData& OldResistanceLightning) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistanceLightning, OldResistanceLightning);
}


void UAuraAttributeSet::OnRep_ResistanceArcane(const FGameplayAttributeData& OldResistanceArcane) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistanceArcane, OldResistanceArcane);
}


void UAuraAttributeSet::OnRep_ResistancePhysical(const FGameplayAttributeData& OldResistancePhysical) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ResistancePhysical, OldResistancePhysical);
}


void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}


void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}


void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}


void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}
