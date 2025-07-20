// Copyright Abdu Inc.


#include "UI/ViewModel/MVVM_LoadMenu.h"

#include "AI/NavigationSystemBase.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadMenu::InitializeLoadSlots()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);
	LoadSlot_0->SetLoadSlotName("LoadSlot_0");
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);
	LoadSlot_1->SetLoadSlotName("LoadSlot_1");
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);
	LoadSlot_2->SetLoadSlotName("LoadSlot_2");

	LoadSlots.Add(0, LoadSlot_0);
	LoadSlots.Add(1, LoadSlot_1);
	LoadSlots.Add(2, LoadSlot_2);

	SetNumSlots(LoadSlots.Num());
}

UMVVM_LoadSlot* UMVVM_LoadMenu::GetLoadSlotViewModelByIndex(int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadMenu::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	AuraGameMode->SaveSlotData(LoadSlots[Slot], Slot);
	LoadSlots[Slot]->InitializeSlot();
}

void UMVVM_LoadMenu::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadMenu::SelectedSlotButtonPressed(int32 Slot)
{
}

void UMVVM_LoadMenu::SetNumSlots(int32 NewNumSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumSlots, NewNumSlots);
}