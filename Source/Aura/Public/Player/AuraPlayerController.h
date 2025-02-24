// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class UAuraAbilitySystemComponent;
struct FGameplayTag;
class UAuraInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AAuraPlayerController();

	virtual void PlayerTick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
	
private:
	void Move(const FInputActionValue& InputActionValue);

	void CursorTrace();

	void AbilityInputTagPressed(FGameplayTag Tag);

	void AbilityInputKeyReleased(FGameplayTag Tag);

	void AbilityInputKeyHeld(FGameplayTag Tag);

	UAuraAbilitySystemComponent* GetAuraAbilitySystemComponent();
	
private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;
	
	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> CurrentActor;
};
