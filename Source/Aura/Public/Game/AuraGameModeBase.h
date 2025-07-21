// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class ULoadMenuSaveGame;
class UMVVM_LoadSlot;
class UAbilityInfo;
class UCharacterClassInfo;
class USaveGame;

/**
 * 
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot);
	ULoadMenuSaveGame* GetSaveSlotData(UMVVM_LoadSlot* LoadSlot);
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);
public:
	/** We store this as a singleton. All enemies' info about their abilities, stats, etc are located here */
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Save")
	TSubclassOf<USaveGame> LoadMenuSaveGameClass;
};
