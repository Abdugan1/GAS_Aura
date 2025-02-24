// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * Used as means to init native GameplayTags
 */
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UAuraAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
