// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
protected:
	/**
	 * Returns spawn locations for Minions.
	 * Could've used a EQS, but that will also do
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<FVector> GetSpawnLocations();

	/** Get Random Minion classes */
	UFUNCTION(BlueprintPure, Category = "Ability")
	TSubclassOf<APawn> GetRandomMinionClass();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	float MinSpawnDistance = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	float MaxSpawnDistance = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ability")
	float SpawnSpreadAngle = 90.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	int32 NumMinions = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TArray<TSubclassOf<APawn>> MinionClasses;
};
