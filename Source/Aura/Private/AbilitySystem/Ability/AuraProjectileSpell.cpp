// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "LevelInstance/LevelInstanceTypes.h"


UAuraProjectileSpell::UAuraProjectileSpell()
{

}

// void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
//                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
//                                            const FGameplayEventData* TriggerEventData)
// {
// 	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
// }


void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation,
	const FGameplayTag& ProjectileSpawnSocketTag)
{
	check(ProjectileClass);
	
	// Spawn projectiles only on the server. Projectiles MUST be replicated so it also appears on clients.
	if (GetAvatarActorFromActorInfo()->HasAuthority())
	{
		const FVector ProjectileSpawnLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			ProjectileSpawnSocketTag
			);
		FRotator ProjectileRotation = (ProjectileTargetLocation - ProjectileSpawnLocation).Rotation();
		// Enemies are shorter than Aura, so there will be a little incline.
		// Setting this to 0 will do the trick
		// UPDATE: It doesn't work for some reason when it's on a dedicated server.
		// ProjectileRotation.Pitch = 0.f;
			
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(ProjectileSpawnLocation);
		SpawnTransform.SetRotation(ProjectileRotation.Quaternion());
			
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.Get()->SetEffectCauser(GetAvatarActorFromActorInfo());
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
		
		const float ScaledDamage = DamageScalableFloat.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageTypeTag, ScaledDamage);
				
		Projectile->DamageEffectSpecHandle = SpecHandle;			
			
		Projectile->FinishSpawning(SpawnTransform);
	}
}

