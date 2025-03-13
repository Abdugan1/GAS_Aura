// Copyright Abdu Inc.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"


UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	// TODO: PlayerController index is 0. Multiplayer issues?
	if (APlayerController *PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PlayerController->GetHUD()))
		{
			AAuraPlayerState *PlayerState = PlayerController->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent *AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
			UAttributeSet *AttributeSet = PlayerState->GetAttributeSet();
			
			const FWidgetControllerParams WidgetControllerParams(PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);
			return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}


UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	// TODO: PlayerController index is 0. Multiplayer issues?
	if (APlayerController *PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AAuraHUD *AuraHUD = Cast<AAuraHUD>(PlayerController->GetHUD()))
		{
			AAuraPlayerState *PlayerState = PlayerController->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent *AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
			UAttributeSet *AttributeSet = PlayerState->GetAttributeSet();

			const FWidgetControllerParams WidgetControllerParams(PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);
			return AuraHUD->GetAttributeMenuController(WidgetControllerParams);
		}
	}
	return nullptr;
}
