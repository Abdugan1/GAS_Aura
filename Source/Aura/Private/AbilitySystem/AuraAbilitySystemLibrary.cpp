// Copyright Abdu Inc.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
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
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

	if (CharacterClassInfo == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterClassInfo is null in InitializeDefaultAttributes(). Called on a Client?"));
		return;
	}
	
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	
	ApplyEffectToASC(AbilitySystemComponent, ClassDefaultInfo.PrimaryAttributes, Level);
	ApplyEffectToASC(AbilitySystemComponent, CharacterClassInfo->SecondaryAttributes, Level);
	ApplyEffectToASC(AbilitySystemComponent, CharacterClassInfo->VitalAttributes, Level);
	
}


void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject,
	UAbilitySystemComponent* AbilitySystemComponent)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterClassInfo is null in GiveStartupAbilities(). Called on a Client?"));
		return;
	}
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1.f);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}


UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraGameMode is null. Called on a Client? GameModes cannot be on Clients."));
		return nullptr;
	}

	return AuraGameMode->CharacterClassInfo;
}


bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// I think no use to check to see if it's a valid pointer, since static_cast doesn't do it -- not dynamic_cast
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->IsBlockedHit();
}


bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// I think no use to check to see if it's a valid pointer, since static_cast doesn't do it -- not dynamic_cast
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->IsCriticalHit();
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
}


void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
}


void UAuraAbilitySystemLibrary::ApplyEffectToASC(UAbilitySystemComponent* AbilitySystemComponent,
                                                 TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level)
{
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(AbilitySystemComponent->GetAvatarActor());
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
