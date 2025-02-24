// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AuraInputConfig.generated.h"

class UInputAction;


USTRUCT(BlueprintType)
struct FAuraInputAction
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	const UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
};

/**
 * For Data-Driven Approach. Each InputAction has its own appropriate GameplayTag so we can identify which button was pressed.
 * On account of that, we can activate correct abilities
 */
UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag InputTag, bool bLogNotFound = false) const;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraInputAction> AbilityInputActions;
};
