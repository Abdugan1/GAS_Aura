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
	CombatSocketLocationTag = FAuraGameplayTags::Get().Montage_Attack_Weapon;
}

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}


void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	check(ProjectileClass);
	check(CombatSocketLocationTag.IsValid());
	
	// Spawn projectiles only on the server. Projectiles MUST be replicated so it also appears on clients.
	if (GetAvatarActorFromActorInfo()->HasAuthority())
	{
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			CombatSocketLocationTag
			);
		FRotator ProjectileRotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		// Enemies are shorter than Aura, so there will be a little incline.
		// Setting this to 0 will do the trick
		// UPDATE: It doesn't work for some reason when it's on a dedicated server.
		// ProjectileRotation.Pitch = 0.f;
			
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

		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), SourceASC->MakeEffectContext());

		for (auto& Pair : DamageTypes)
		{
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}
				
		Projectile->DamageEffectSpecHandle = SpecHandle;			
			
		Projectile->FinishSpawning(SpawnTransform);
	}
}
