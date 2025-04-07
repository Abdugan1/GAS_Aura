#pragma once
#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"

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

	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit() const { return bIsBlockedHit; }

	void SetIsCriticalHit(bool bNewCriticalHit) { bIsCriticalHit = bNewCriticalHit; }
	void SetIsBlockedHit(bool bNewBlockedHit) { bIsBlockedHit = bNewBlockedHit; }
	
protected:
	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsBlockedHit = false;
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