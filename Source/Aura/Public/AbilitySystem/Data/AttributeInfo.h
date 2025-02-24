// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AttributeInfo.generated.h"

/**
 * Used for updating Attribute Menu. Everytime an attribute changes,
 * AttributeMenuWidgetController sends this with AttributeValue deviated.
 */
USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription;

	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute Attribute;
};

/**
 * DataAsset to assign every attribute need to its own GameplayTag.
 * So it's easier to find an attribute changed
 */
UCLASS()
class AURA_API UAttributeInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	FAuraAttributeInfo FindAttributeInfoForTag(const FGameplayTag AttributeTag, bool bLogNotFound = false) const;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraAttributeInfo> AttributeInfos;
};
