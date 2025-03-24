// Copyright Abdu Inc.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Game/AuraGameModeBase.h"
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


void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject,
	ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* AbilitySystemComponent)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		return;
	}

	UCharacterClassInfo* CharacterClassInfo = AuraGameMode->CharacterClassInfo;

	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	
	ApplyEffectToASC(AbilitySystemComponent, ClassDefaultInfo.PrimaryAttributes, Level);
	ApplyEffectToASC(AbilitySystemComponent, CharacterClassInfo->SecondaryAttributes, Level);
	ApplyEffectToASC(AbilitySystemComponent, CharacterClassInfo->VitalAttributes, Level);
	
}


void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject,
	UAbilitySystemComponent* AbilitySystemComponent)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		return;
	}

	UCharacterClassInfo* CharacterClassInfo = AuraGameMode->CharacterClassInfo;
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1.f);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}


void UAuraAbilitySystemLibrary::ApplyEffectToASC(UAbilitySystemComponent* AbilitySystemComponent,
                                                 TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level)
{
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(AbilitySystemComponent->GetAvatarActor());
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
