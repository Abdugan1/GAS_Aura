// Copyright Abdu Inc.


#include "Actor/AuraProjectile.h"


#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_Projectile);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetSphereRadius(10);

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComponent");
	MovementComponent->InitialSpeed = 550.f;
	MovementComponent->MaxSpeed = 550.f;
	MovementComponent->ProjectileGravityScale = 0.f;

	LoopAudioComponent = CreateDefaultSubobject<UAudioComponent>("LoopComponent");
	LoopAudioComponent->SetupAttachment(GetRootComponent());	
}

void AAuraProjectile::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	if (LoopAudioComponent) LoopAudioComponent->Stop();
	bHit = true;
}

void AAuraProjectile::Destroyed()
{
	if (!bHit && !HasAuthority())
	{
		OnHit();
	}
	Super::Destroyed();
}


void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);

	// Without this, homing projectiles' movement doesn't replicate!
	SetReplicateMovement(true);
}


void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
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
			const FVector KnockbackImpulse = GetActorForwardVector() * DamageEffectParams.KnockbackMagnitude;
			
			/** Now we know the TargetASC so set it */
			DamageEffectParams.TargetAbiltySystemComponent = TargetASC;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			DamageEffectParams.KnockbackImpulse = KnockbackImpulse;
			
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		
		Destroy();
	}
	else
	{
		bHit = true;
	}



	//
	// BELOW IS MY SOLUTION. IT's IN ORDER! DO NOT CHANGE ANYTHING!!! JUST UNCOMMENT!
	//
	
	
	// UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	// UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	//
 //    SetActorHiddenInGame(true);
	// SetActorEnableCollision(false);
	
	// if (HasAuthority())
	// {
	// 	SetLifeSpan(2);
	// 	
	// 	// No friendly fire allowed
	// 	// I think we can also use Owner instead of DamageEffectSpecHandle.GetContext().GetEffectCauser()
	// 	// TODO: What happens if an Enemy had shot a projectile, but then we killed that enemy? Should I use some kind of struct to store necessary data?
	// 	if (!UAuraAbilitySystemLibrary::IsNotFriend(DamageEffectSpecHandle.Data->GetContext().GetEffectCauser(), OtherActor))
	// 	{
	// 		return;
	// 	}
	// 	if (UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	// 	{
	// 		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
	// 	}
	// }
}


