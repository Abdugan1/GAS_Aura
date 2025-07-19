// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LoadMenuHUD.generated.h"

class UMVVM_LoadMenu;
class ULoadMenuWidget;

/**
 * 
 */
UCLASS()
class AURA_API ALoadMenuHUD : public AHUD
{
	GENERATED_BODY()
public:

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadMenuWidgetClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULoadMenuWidget> LoadMenuWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadMenu> LoadMenuViewModelClass;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_LoadMenu> LoadMenuViewModel;
};
