// Copyright Abdu Inc.


#include "UI/ViewModel/MVVM_LoadMenu.h"

#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadMenu::InitializeLoadSlots()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);

	LoadSlots.Add(0, LoadSlot_0);
	LoadSlots.Add(1, LoadSlot_1);
	LoadSlots.Add(2, LoadSlot_2);	
}

UMVVM_LoadSlot* UMVVM_LoadMenu::GetLoadSlotViewModelByIndex(int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadMenu::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
}

void UMVVM_LoadMenu::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadMenu::SelectedSlotButtonPressed(int32 Slot)
{
}