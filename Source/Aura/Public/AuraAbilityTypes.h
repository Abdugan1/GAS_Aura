#pragma once
#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()
	FDamageEffectParams() = default;

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject = nullptr;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetAbiltySystemComponent;

	UPROPERTY()
	float BaseDamage = 0.f;

	UPROPERTY()
	float AbilityLevel = 1.f;

	UPROPERTY()
	FGameplayTag DamageTypeTag;

	UPROPERTY()
	float DebugChance = 0.f;

	UPROPERTY()
	float DebuffDamage = 0.f;

	UPROPERTY()
	float DebuffDuration = 0.f;

	UPROPERTY()
	float DebuffFrequency = 0.f;

	UPROPERTY()
	float DeathImpulseMagnitude = 0.f;

	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
public:
	//** BOILERPLATE */
	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const
	{
		// Doesn't work FGameplayEffectContext::StaticStruct()... or I think so.
		// Works fine with plain StaticStruct()
		return StaticStruct(); 
	}

	virtual FGameplayEffectContext* Duplicate() const override;
	
	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
	//** end BOILERPLATE */

	/** Is it a critical hit?  */
	bool IsCriticalHit() const { return bIsCriticalHit; }

	/** Was it blocked? */
	bool IsBlockedHit() const { return bIsBlockedHit; }

	bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }

	float GetDebuffDamage() const { return DebuffDamage; }

	float GetDebuffDuration() const { return DebuffDuration; }

	float GetDebuffFrequency() const { return DebuffFrequency; }

	FGameplayTag GetDamageTypeTag() const { return DamageTypeTag; }

	FVector GetDeathImpulse() const { return DeathImpulse; }
	
	void SetIsSuccessfulDebuff(bool bInIsSuccessfulDebuff) { bIsSuccessfulDebuff = bInIsSuccessfulDebuff; }
	void SetIsCriticalHit(bool bNewCriticalHit) { bIsCriticalHit = bNewCriticalHit; }
	void SetIsBlockedHit(bool bNewBlockedHit) { bIsBlockedHit = bNewBlockedHit; }
	void SetDebuffDamage(float NewDamage) { DebuffDamage = NewDamage; }
	void SetDebuffDuration(float NewDuration) { DebuffDuration = NewDuration; }
	void SetDebuffFrequency(float NewFreq) { DebuffFrequency = NewFreq; }
	void SetDamageTypeTag(const FGameplayTag& NewTag) { DamageTypeTag = NewTag; }
	void SetDeathImpulse(const FVector& NewImpulse) { DeathImpulse = NewImpulse; }
	
protected:
	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsSuccessfulDebuff = false;

	UPROPERTY()
	float DebuffDamage = 0.f;

	UPROPERTY()
	float DebuffDuration = 0.f;

	UPROPERTY()
	float DebuffFrequency = 0.f;

	UPROPERTY()
	FGameplayTag DamageTypeTag;

	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;
};

/** BOILERPLATE */
template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true,
	};
};
/** end BOILERPLATE */