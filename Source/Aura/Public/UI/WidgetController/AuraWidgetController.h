// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

class UAuraAbilitySystemComponent;
class AAuraPlayerState;
class AAuraPlayerController;
class UAuraAttributeSet;
class UAbilitySystemComponent;
class UAttributeSet;
struct FAuraAbilityInfo;
class UAbilityInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FAuraAbilityInfo&, Info);

/**
 * 
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()
public:
	FWidgetControllerParams() {}
	FWidgetControllerParams(APlayerController *PC, APlayerState* PS, UAbilitySystemComponent *ASC, UAttributeSet *AS)
		: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};


/**
 * 
 */
UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams &WidgetControllerParams);

	/**
	 * This is necessary to because initial values DO NOT trigger any events.
	 * So just to keep our UI up-to-date, we need to broadcast it manually.
	 * All child classes must broadcast on their own.
	 *
	 * Called in Aura
	 */
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();

	/**
	 * Bind necessary ASC's attributes to callbacks, broadcasts.
	 * All child classes must do it on their own.
	 */
	virtual void BindCallbacksToDependencies();

	/**
	 * Broadcast Initial Abilities' Info to init Overlay's Skill Globes, i.e., what slot, what icon and background.
	 */
	void BroadcastInitialAbilitiesInfo();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilitiesInfo;
};
