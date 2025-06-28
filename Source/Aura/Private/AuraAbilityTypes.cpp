#include "AuraAbilityTypes.h"


FGameplayEffectContext* FAuraGameplayEffectContext::Duplicate() const
{
	// Don't have any idea what's going on. Just need this after UE5.3 I guess.
	FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
	{
		// Does a deep copy of the hit result
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// Until RepBits |= 1 << 6;, it's all just boilerplate "stolen" from the actual FGameplayEffectContext source code.
	// All I did was this
	// if (bIsCriticalHit)
	// {
	// 	RepBits |= 1 << 7;
	// }
	// if (bIsBlockedHit)
	// {
	// 	RepBits |= 1 << 8;
	// }
	// And this
	// if (RepBits & (1 << 7))
	// {
	// 	bIsCriticalHit = true;
	// }
	// if (RepBits & (1 << 8))
	// {
	// 	bIsBlockedHit = true;
	// }
	uint32 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		// Below is mine
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 7;
		}
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 8;
		}
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 9;
		}
		if (!FMath::IsNearlyZero(DebuffDamage))
		{
			RepBits |= 1 << 10;
		}
		if (!FMath::IsNearlyZero(DebuffDuration))
		{
			RepBits |= 1 << 11;
		}
		if (!FMath::IsNearlyZero(DebuffFrequency))
		{
			RepBits |= 1 << 12;
		}
		if (DamageTypeTag.IsValid())
		{
			RepBits |= 1 << 13;
		}
		if (!DeathImpulse.IsZero())
		{
			RepBits |= 1 << 14;
		}
	}

	Ar.SerializeBits(&RepBits, 15);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}

	// Below is mine
	if (RepBits & (1 << 7))
	{
		// I think I can use either Ar << or just assigning 
		bIsCriticalHit = true;
	}
	if (RepBits & (1 << 8))
	{
		// I think I can use either Ar << or just assigning 
		bIsBlockedHit = true;
	}
	if (RepBits & (1 << 9))
	{
		// I think I can use either Ar << or just assigning 
		Ar << bIsSuccessfulDebuff;
	}
	if (RepBits & (1 << 10))
	{
		// I think I can use either Ar << or just assigning 
		Ar << DebuffDamage;
	}
	if (RepBits & (1 << 11))
	{
		Ar << DebuffDuration;
	}
	if (RepBits & (1 << 12))
	{
		Ar << DebuffFrequency;
	}
	if (RepBits & (1 << 13))
	{
		/**
		 * NOTE: Stepehen here used the same method to serizlize that was used for HitResult.
		 * I guess, maybe, Stephen doesn't know that you can use just plain Ar << as the operator allows it.
		 * Anyway, if any errors occur... Well...
		 */
		Ar << DamageTypeTag;
	}
	if (RepBits & (1 << 14))
	{
		// Note: Again, for some reason, Stephen uses this instead of Ar << DeathImpulse
		DeathImpulse.NetSerialize(Ar, Map, bOutSuccess);
		
	}

	// end mine
	
	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}	
	
	bOutSuccess = true;
	return true;
}
