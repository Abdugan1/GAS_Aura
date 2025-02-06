// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

struct FWidgetControllerParams;
class UOverlayWidgetController;
class UAuraUserWidget;
class UAbilitySystemComponent;
class UAttributeSet;

/**
 * 
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams &WidgetControllerParams);

	/**
	 * Inits the overlay(HUD). We need these params to pass them into the controller.
	 */
	void InitOverlay(APlayerController *PC, APlayerState* PS, UAbilitySystemComponent *ASC, UAttributeSet *AS);
	
public:
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;
	
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
};
