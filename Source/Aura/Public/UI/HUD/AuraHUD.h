// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

class UAttributeMenuWidgetController;
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

	UAttributeMenuWidgetController* GetAttributeMenuController(const FWidgetControllerParams &WidgetControllerParams);

	/**
	 * Inits the overlay(HUD). We need these params to pass them into the controller.
	 */
	void InitOverlay(APlayerController *PC, APlayerState* PS, UAbilitySystemComponent *ASC, UAttributeSet *AS);
	
private:
	//** Overlay Widget's */
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
	//** end Overlay Widget's */

	//** Attribute Menu's *
	UPROPERTY()
	TObjectPtr<UAttributeMenuWidgetController> AttributeMenuController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuControllerClass;
	//** end Attribute Menu's *
	
};
