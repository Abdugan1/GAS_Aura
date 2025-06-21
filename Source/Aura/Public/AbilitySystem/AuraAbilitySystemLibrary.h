// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UCharacterClassInfo;
class UGameplayEffect;
class UAttributeMenuWidgetController;
class UOverlayWidgetController;
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * Returns Overlay Widget Controller. It's basically a Singleton.
	 * Once created, then returns the same object every time
	 * Return from HUD.
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/**
	 * Returns Attribute Menu Widget Controller. It's basically a Singleton.
	 * Once created, then returns the same object every time
	 * Return from HUD.
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	/**
	 * Returns Spell Menu Widget Controller. It's basically a Singleton.
     * Once created, then returns the same object every time
	 * Return from HUD.
	 */
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);
	
	/** Initializes a given ASC's (character's) default attributes using a special DataAsset */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|WidgetController")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* AbilitySystemComponent);

	/** Initializes a given ASC's (character's) default abilities using a special DataAsset */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|WidgetController")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* AbilitySystemComponent, ECharacterClass CharacterClass);

	/** A convinient function to return the singleton UCharacterClassInfo from the GameMode class */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|WidgetController")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	/** A convinient function to return the singleton UAbilityInfo from the GameMode class */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|WidgetController")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetLiveActorsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);	

	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);
	
private:
	static void ApplyEffectToASC(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level);
	static FWidgetControllerParams GetWidgetControllerParams(const UObject* WorldContextObject, APlayerController* PlayerController);
};
