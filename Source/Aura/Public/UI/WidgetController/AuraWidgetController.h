// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION(BlueprintPure)
	APlayerController *GetPlayerController();
	
	const APlayerController *GetPlayerController() const;
	
	UFUNCTION(BlueprintPure)
	APlayerState *GetPlayerState();

	const APlayerState *GetPlayerState() const;

	UFUNCTION(BlueprintPure)
	UAbilitySystemComponent *GetAbilitySystemComponent();

	const UAbilitySystemComponent *GetAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure)
	UAttributeSet *GetAttributeSet();

	const UAttributeSet *GetAttributeSet() const;
	
	AAuraPlayerController *GetAuraPlayerController();
	AAuraPlayerState *GetAuraPlayerState();
	UAuraAbilitySystemComponent *GetAuraAbilitySystemComponent();
	UAuraAttributeSet *GetAuraAttributeSet();
	
	
protected:
	UPROPERTY(BlueprintAssignable, Category = "GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilitiesInfo;

private:
   	UPROPERTY()
   	TObjectPtr<APlayerController> PlayerController;
   
   	UPROPERTY()
   	TObjectPtr<APlayerState> PlayerState;
   
   	UPROPERTY()
   	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
   
   	UPROPERTY()
   	TObjectPtr<UAttributeSet> AttributeSet;
   
   	UPROPERTY()
   	TObjectPtr<AAuraPlayerController> AuraPlayerController;
   
   	UPROPERTY()
   	TObjectPtr<AAuraPlayerState> AuraPlayerState;
   
   	UPROPERTY()
   	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
   
   	UPROPERTY()
   	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;
};
