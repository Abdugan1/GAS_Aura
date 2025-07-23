// Copyright Abdu Inc.


#include "Game/AuraGameModeBase.h"

#include "Game/AuraGameInstance.h"
#include "Game/LoadMenuSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void AAuraGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot)
{
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), LoadSlot->SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), LoadSlot->SlotIndex);
	}
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadMenuSaveGameClass);
	
	ULoadMenuSaveGame* LoadMenuSaveGame = Cast<ULoadMenuSaveGame>(SaveGameObject);
	LoadMenuSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadMenuSaveGame->SlotStatus = Taken; // Since we're saving - it's Taken
	LoadMenuSaveGame->MapName = LoadSlot->GetMapName();
	LoadMenuSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;
	
	UGameplayStatics::SaveGameToSlot(LoadMenuSaveGame, LoadSlot->GetLoadSlotName(), LoadSlot->SlotIndex);
}

ULoadMenuSaveGame* AAuraGameModeBase::GetSaveSlotData(UMVVM_LoadSlot* LoadSlot)
{
	USaveGame* SaveGameObject = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), LoadSlot->SlotIndex))
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(LoadSlot->GetLoadSlotName(), LoadSlot->SlotIndex);
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadMenuSaveGameClass);
	}
	return Cast<ULoadMenuSaveGame>(SaveGameObject);
}

void AAuraGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

void AAuraGameModeBase::TravelToMap(UMVVM_LoadSlot* LoadSlot)
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(LoadSlot, Maps.FindChecked(LoadSlot->GetMapName()));
}

AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(UGameplayStatics::GetGameInstance(this));
	
	TArray<AActor*> PlayerStartActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartActors);
	if (PlayerStartActors.Num() > 0)
	{
		AActor* SelectedPlayerStartActor = PlayerStartActors[0];
		for (AActor* PlayerStartActor : PlayerStartActors)
		{
			APlayerStart* PlayerStart = Cast<APlayerStart>(PlayerStartActor);
			if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
			{
				SelectedPlayerStartActor = PlayerStart;
				break;
			}
		}
		return SelectedPlayerStartActor;
	}
	return nullptr;
}

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	Maps.Add(DefaultMapName, DefaultMap);
}
