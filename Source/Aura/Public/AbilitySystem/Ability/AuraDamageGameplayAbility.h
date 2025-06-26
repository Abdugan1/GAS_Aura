// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "Interaction/CombatInterface.h"
#include "AuraDamageGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor = nullptr) const;
protected:
	/** Convenient function to cause damage to a given actor. The actor must hold ASC */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void CauseDamage(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category = "Damage")
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& MontageArray) const;

	float GetManaCost(float Level) const;
	float GetCooldown(float Level) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta=(Categories="Damage"))
	FGameplayTag DamageTypeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FScalableFloat DamageScalableFloat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DebuffChance = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DebuffDamage = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DebuffFrequency = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DebuffDuration = 5.f;
};
