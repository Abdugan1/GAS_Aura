// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AuraDamageGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;
/**
 * 
 */
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
	UAuraProjectileSpell();
	
protected:
	/** For now does nothing */
	// virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * Spawns a projectile of the ProjectileClass instance.
	 * Applies DamageEffectClass to the projectile
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& ProjectileSpawnSocketTag);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AAuraProjectile> ProjectileClass;
};
