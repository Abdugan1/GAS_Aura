// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/AuraProjectileSpell.h"
#include "AuraFireBolt.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraFireBolt : public UAuraProjectileSpell
{
	GENERATED_BODY()
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

private:
	FString GetDescriptionInternal(const FString& Title,int32 Level) const;
};
