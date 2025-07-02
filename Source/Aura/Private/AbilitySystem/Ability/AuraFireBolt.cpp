// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraFireBolt.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"


FString UAuraFireBolt::GetDescription(int32 Level)
{
	return GetDescriptionInternal("FIRE BOLT", Level);
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	return GetDescriptionInternal("NEXT LEVEL", Level);
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation,
	const FGameplayTag& ProjectileSpawnSocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	// check(ProjectileClass);
	
	// Spawn projectiles only on the server. Projectiles MUST be replicated so it also appears on clients.
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}
	
	const FVector ProjectileSpawnLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			ProjectileSpawnSocketTag
			);
	FRotator ProjectileRotation = (ProjectileTargetLocation - ProjectileSpawnLocation).Rotation();
	if (bOverridePitch)
	{
		ProjectileRotation.Pitch = PitchOverride;
	}

	const int32 NumProjectiles = FMath::Min(MaxNumberOfProjectiles, GetAbilityLevel());
		
	const FVector Forward = ProjectileRotation.Vector();
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectiles);

	for (const FRotator& Rotation : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(ProjectileSpawnLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());

		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults(
			/** We don't know who the target is, only when the projectile hits someone will the projectile set the TargetASC */
			);

		if (IsValid(HomingTarget) && HomingTarget->Implements<UCombatInterface>())
		{
			Projectile->MovementComponent->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->MovementComponent->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}

		Projectile->MovementComponent->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->MovementComponent->bIsHomingProjectile = bIsHomingProjectile;
		
		Projectile->FinishSpawning(SpawnTransform);
	}


}

FString UAuraFireBolt::GetDescriptionInternal(const FString& Title, int32 Level) const
{
	const int32 AbilityDamage = FMath::RoundHalfToEven(DamageScalableFloat.GetValueAtLevel(Level));
	const int32 FireBoltsAmount = Level;
	const float ManaCost = GetManaCost(Level);
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		"<Title>%s</>\n\n"
		
		"<Small>Level: </><Level>%i</>\n"
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Launches %i bolts of fire, exploding on impact and dealing: </>"
		"<Damage>%i</> <Default>fire damage with a chance to burn</>\n\n"
		), *Title, Level, ManaCost, Cooldown, FireBoltsAmount, AbilityDamage);
}
