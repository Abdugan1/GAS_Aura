// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UNiagaraSystem;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);

/**
 * Struct to hold a Montage bound to a particular GameplayTag - MontageTag.
 * Used to make Attack Montages easier to deal with.
 * SocketTag is the tag the damage will be caused or a projectile will be cast.
 * ImpactSound is the impact sound when it caused any damage - useless for ranged attackers since projectiles must to that 
 */
USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
	
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Used to get actors level. No matter enemy or player.
	 * NOTE: Could not name this since GAS already has GetActorLevel function
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetPlayerLevel();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetCombatSocketLocation(const FGameplayTag& SocketTag);

	/**
	 * Used along with the MotionWarping system.
	 * In this project, used face the Offender to the Target whenever it wants to deal damage
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetFacingTarget(const FVector& FacingTarget);

	/** Return the hit montage to play whenever gets hit */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	/** To die */
	virtual void Die(const FVector& InDeathImpulse) = 0;

	/** If the actor is dead */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDead() const;

	/** Mainly used with HitReact. Gets the Blood Effect whenever gets hurt */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UNiagaraSystem* GetBloodEffect();

	/** Return the avatar */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();

	/** Return all the available attack montages */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages();
	
	/** Get Montage by Tag */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);

	/** How many minions/companions do I have? Could be a Shamans demons or Aura's pets, if has one */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMinionCount();

	/** Modify minion count. If the amount is negative, it decrements */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncrementMinionCount(int32 Amount);

	/** Get the Character class */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ECharacterClass GetCharacterClass() const;

	// For some reason, you can, or can you, return a copy of it
	// But GetOnDeathm you cannot do that...
	virtual FOnASCRegistered GetOnASCRegistered() = 0;

	// But 
	virtual FOnDeath* GetOnDeath() = 0;
};
