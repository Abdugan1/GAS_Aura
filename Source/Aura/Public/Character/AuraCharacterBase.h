// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UDebuffNiagaraComponent;
class UNiagaraSystem;
class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** ICombatInterface */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	
	virtual void Die(const FVector& InDeathImpulse) override;

	virtual bool IsDead_Implementation() const override;
	
	virtual AActor* GetAvatar_Implementation() override;

	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;

	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;

	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;

	virtual int32 GetMinionCount_Implementation() override;

	virtual void IncrementMinionCount_Implementation(int32 Amount) override;

	virtual ECharacterClass GetCharacterClass_Implementation() const override;

	virtual FOnASCRegistered GetOnASCRegistered() override;
	virtual FOnDeath* GetOnDeath() override;

	virtual void ApplyKnockback(const FVector& KnockbackImpulse) override;

	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;

	virtual bool IsBeingShocked_Implementation() const override;
	virtual void SetIsBeingShocked_Implementation(bool InIsBeingShocked) override;
	
	/** end ICombatInterface */

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& InDeathImpulse);

	FOnASCRegistered OnASCRegistered;
	
	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	/**
	 * Meant to init ASC->InitAbilityActorInfo.
	 * Users must call and implement this function on their own
	 * as it's not called anywhere on the base class and is empty - does nothing.
	 */
	virtual void InitAbilityActorInfo();

	/**
	 * A convenient function to apply gameplay effects to self 
	 */
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	/** Initializes default attributes. */
	virtual void InitializeDefaultAttributes() const;

	/**
	 * Add default abilities to self. Only the server adds them. Then the GAS replication system takes care of the rest
	 * NOTE: Adds both plain Startup Abilities and PASSIVE Startup Abilities
	 */
	void AddCharacterAbilities() const;

	/** ICombatInterface */
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag) override;
	/** end ICombatInterface */

	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartMeshDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/** RepNotifies */
	UFUNCTION()
	virtual void OnRep_IsStunned(bool OldIsStunned) const;
	/** end RepNotifies */

	virtual void IsStunnedChanged(const FGameplayTag Callbacktag, int32 NewCount);
	
public:
	/**
	 * Stephen also created bIsBurned var, I'm too lazy to do that.
	 * It's needed to fix AuraCharacter on clients.
	 * */
	
	UPROPERTY(ReplicatedUsing=OnRep_IsStunned, BlueprintReadOnly, Category= "Combat")
	bool bIsStunned = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category= "Combat")
	bool bIsBeingShocked = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 650.f;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName TailSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USoundBase* DeathSound;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat")
	UNiagaraSystem* BloodEffect;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY()
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	/* Dissolve effects */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInstance> MeshDissolveMaterialInstanceClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Materials")
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstanceClass;

	/* Minions */
	int32 MinionCount = 0;
	
private:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;
};
