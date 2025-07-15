// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Aura/AuraLogChannels.h"


FDamageEffectParams UAuraDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor, FVector RadialDamageOrigin) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbiltySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Params.BaseDamage = DamageScalableFloat.GetValueAtLevel(GetAbilityLevel());
	Params.AbilityLevel = GetAbilityLevel();
	Params.DamageTypeTag = DamageTypeTag;
	Params.DebuffChance = DebuffChance;
	Params.DebuffDamage = DebuffDamage;
	Params.DebuffFrequency = DebuffFrequency;
	Params.DebuffDuration = DebuffDuration;
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	/**
	 * Note on DeathImpulse.
	 * Setting DeathImpulse is not possible at the beginning
	 * as the projectile that hit an enemy defines the impulse/direction.
	 */
	Params.KnockbackMagnitude = KnockbackMagnitude;
	/**
	 * Note on KnockbackImpulse.
	 * Setting DeathImpulse is not possible at the beginning
	 * as the projectile that hit an enemy defines the impulse/direction.
	 */
	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
		Params.RadialDamageOrigin = RadialDamageOrigin;
	}
	return Params;
}

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
   	{
   		UE_LOG(LogAura, Warning, TEXT("CauseDamage: Not Authoritative"));
   		return;
   	}
	
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);


	float Damage = DamageScalableFloat.GetValueAtLevel(GetAbilityLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, DamageTypeTag, Damage);

	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}


FTaggedMontage UAuraDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& MontageArray) const
{
	if (MontageArray.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("MontageArray is empty. Did forget to assign attack animations for %s?"), *GetAvatarActorFromActorInfo()->GetName());
		return FTaggedMontage{};
	}

	const auto Selection = FMath::RandRange(0, MontageArray.Num() - 1);
	return MontageArray[Selection];
}

float UAuraDamageGameplayAbility::GetManaCost(float Level) const
{
	float ManaCost = 0.0f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if (Mod.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(Level, ManaCost);
				break;
			}
		}
	}

	return ManaCost;
}

float UAuraDamageGameplayAbility::GetCooldown(float Level) const
{
	float Cooldown = 0.0f;
	if (UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		// No Modifiers in CooldownEffect since it's just a duration based effect
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(Level, Cooldown);
	}
	return Cooldown;
}
