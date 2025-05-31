// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"

USTRUCT(BlueprintType)
struct FAuraLevelUpInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 LevelUpRequirement{0};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AttributePointReward{1};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpellPointReward{1};
};

/**
 * 
 */
UCLASS()
class AURA_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	int32 FindLevelForXP(int32 XP);
public:
	UPROPERTY(EditAnywhere)
	TArray<FAuraLevelUpInfo> LevelUpInfos;
};
