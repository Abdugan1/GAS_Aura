// Copyright Abdu Inc.


#include "AbilitySystem/Data/AbilityInfo.h"

#include "Aura/AuraLogChannels.h"

FAuraAbilityInfo UAbilityInfo::FindAbilityInfoFromTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const auto& AbilityInfo : AbilityInformation)
	{
		if (AbilityInfo.AbilityTag == AbilityTag)
		{
			return AbilityInfo;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("No such ability info [%s]"), *AbilityTag.ToString());
	}
	
	return FAuraAbilityInfo();
}
