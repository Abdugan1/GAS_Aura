// Copyright Abdu Inc.


#include "Game/AuraGameModeBase.h"

#include "Game/LoadMenuSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void AAuraGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
	USaveGame* SaveGameObject =  UGameplayStatics::CreateSaveGameObject(LoadMenuSaveGameClass);
	ULoadMenuSaveGame* LoadMenuSaveGame = Cast<ULoadMenuSaveGame>(SaveGameObject);
	LoadMenuSaveGame->PlayerName = LoadSlot->GetPlayerName();
	UGameplayStatics::SaveGameToSlot(LoadMenuSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}
