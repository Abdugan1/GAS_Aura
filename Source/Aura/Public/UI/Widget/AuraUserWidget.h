// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * Design based. User created WBPs should have their own widget controller.
 * The controller is responsible to retrieve and serve data.
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;
	
protected:
	/** Triggered when WidgetController is set */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
