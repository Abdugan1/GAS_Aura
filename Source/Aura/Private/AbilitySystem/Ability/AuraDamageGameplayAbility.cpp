// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"


void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	auto SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);

	for (auto DamageType : DamageTypes)
	{
		float Damage = DamageType.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType.Key, Damage);
	}

	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
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
