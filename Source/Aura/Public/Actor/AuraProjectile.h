// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "AuraProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UNiagaraSystem;

UCLASS()
class AURA_API AAuraProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraProjectile();
	UFUNCTION(BlueprintCallable)
	void OnHit();

	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> MovementComponent;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	FDamageEffectParams DamageEffectParams;

	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;
	
private:
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 8.f;

	bool bHit = false;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(EditAnywhere, Category = "Projectile|Impact")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditAnywhere, Category = "Projectile|Impact")
	TObjectPtr<USoundBase> ImpactSound;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UAudioComponent* LoopAudioComponent;
};
