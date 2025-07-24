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
	ULoadMenuSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex);
	ULoadMenuSaveGame* RetrieveInGameSaveData();
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);
	void SaveInGameProgressData(ULoadMenuSaveGame* SaveData);

	void TravelToMap(UMVVM_LoadSlot* LoadSlot);
	
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
protected:
	virtual void BeginPlay() override;
	
public:
	/** We store this as a singleton. All enemies' info about their abilities, stats, etc are located here */
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Save")
	TSubclassOf<USaveGame> LoadMenuSaveGameClass;

	UPROPERTY(EditDefaultsOnly, Category = "Maps")
	FString DefaultMapName;

	UPROPERTY(EditDefaultsOnly, Category = "Maps")
	TSoftObjectPtr<UWorld> DefaultMap;

	UPROPERTY(EditDefaultsOnly, Category = "Maps")
	FName DefaultPlayerStartTag;

	UPROPERTY(EditDefaultsOnly, Category = "Maps")
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;
};
