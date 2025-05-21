// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * This is used to use my own FAuraGameplayEffectContext in the entire project. We also must use it in the editor.
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
