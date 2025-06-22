// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, const FString&, DescriptionText, const FString&, NextLevelDescriptionText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityTypeTag);

struct FSelectedAbility
{
	FGameplayTag AbilityTag = FGameplayTag();
	FGameplayTag StatusTag = FGameplayTag();
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();

	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature OnPlayerSpellPointsChanged;

	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature OnSpellGlobeSelected;

	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature WaitForEquipSelectionDelegate;

	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature StopWaitForEquipSelectionDelegate;
	
private:
	/** SelectedAbility MUST be set appropriately */
	void CheckAndBroadcastOnSpellGlobeSelected();
	
private:
	FSelectedAbility SelectedAbility;
	bool bWaitingForEquipSelection = false;
};
