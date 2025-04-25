// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


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
