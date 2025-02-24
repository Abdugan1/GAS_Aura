// Copyright Abdu Inc.


#include "Input/AuraInputConfig.h"


const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag InputTag, bool bLogNotFound) const
{
	for (const FAuraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find AbilityInputAction for %s."), *InputTag.ToString());
	}
	
	return nullptr;
}
