// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"

class UGameplayAbility;

/**
 * A struct to in the main overlay. To show the icon and the background material.
 * Users must specify AbilityTag, Icon, and BackgroundMaterial in the editor.
 * InputTag is assigned with code. One can define StartupInputTag of any AuraGameplayAbility in the editor. 
 */
USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag CooldownTag;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<const UTexture2D> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<const UMaterialInterface> BackgroundMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 LevelUpRequirement = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass;
};

/**
 * Holds all FAuraAbilityInfos. 
 */
UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="AbilityInformation")
	TArray<FAuraAbilityInfo> AbilityInformation;

	/**
	 * Returns FAuraAbilityInfo of the given AbilityTag. Returns empty FAuraAbilityInfo if not found.
	 */
	FAuraAbilityInfo FindAbilityInfoFromTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;
};
