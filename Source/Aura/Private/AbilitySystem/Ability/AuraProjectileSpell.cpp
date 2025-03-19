// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraProjectileSpell.h"

#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "LevelInstance/LevelInstanceTypes.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}


void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	check(ProjectileClass);
	
	// Spawn projectiles only on the server. Projectiles MUST be replicated so it also appears on clients.
	if (GetAvatarActorFromActorInfo()->HasAuthority())
	{
		ICombatInterface *CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
		if (CombatInterface)
		{
			const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
			 FRotator ProjectileRotation = (ProjectileTargetLocation - SocketLocation).Rotation();
			// Enemies are shorter than Aura, so there will be a little incline.
			// Setting this to 0 will do the trick
			ProjectileRotation.Pitch = 0.f;
			
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SocketLocation);
			SpawnTransform.SetRotation(ProjectileRotation.Quaternion());
			
			AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
				ProjectileClass,
				SpawnTransform,
				GetOwningActorFromActorInfo(),
				Cast<APawn>(GetOwningActorFromActorInfo()),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);

			//TODO: Give the Projectile a Gameplay Effect Spec for causing Damage.
			
			Projectile->FinishSpawning(SpawnTransform);
		}
	}
}
