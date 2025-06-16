// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * @class UAuraGameplayAbility
 * @brief Base class for all gameplay abilities in the Aura project.
 *
 * This class extends UGameplayAbility to add project-specific properties,
 * such as a StartupInputTag for associating abilities with input actions.
 */
UCLASS() // UCLASS macro is required for Unreal Engine reflection
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY() // This macro MUST be the first thing inside the class body.

public:
	/**
	 * @brief The InputTag this Ability is starts with.
	 * In this project, action buttons are bound with GameplayTags.
	 * This tag will be added to the Ability's DynamicAbilityTags when the ability is given.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

	// You might also want to override virtual functions from UGameplayAbility here,
	// e.g., ActivateAbility, CanActivateAbility, etc.
};

