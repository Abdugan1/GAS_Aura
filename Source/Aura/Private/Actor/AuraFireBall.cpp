// Copyright Abdu Inc.


#include "Actor/AuraFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

void AAuraFireBall::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

void AAuraFireBall::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Avoid self-hit	
   	if (GetOwner() == OtherActor)
   	{
   		return;
   	}

	if (!bHit)
	{
		OnHit();
	}

	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			// No knockback here.
			
			/** Now we know the TargetASC so set it */
			DamageEffectParams.TargetAbiltySystemComponent = TargetASC;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}	
}
