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
	LoadSlot_0->SlotIndex = 0;
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);
	LoadSlot_1->SetLoadSlotName("LoadSlot_1");
	LoadSlot_1->SlotIndex = 1;
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, UMVVM_LoadSlot_Class);
	LoadSlot_2->SetLoadSlotName("LoadSlot_2");
	LoadSlot_2->SlotIndex = 2;

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

	LoadSlots[Slot]->SetMapName(AuraGameMode->DefaultMapName);
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->SlotStatus = Taken;
	
	AuraGameMode->SaveSlotData(LoadSlots[Slot]);
	
	LoadSlots[Slot]->InitializeSlot();
}

void UMVVM_LoadMenu::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SlotStatus = EnterName;
	LoadSlots[Slot]->InitializeSlot();
}

void UMVVM_LoadMenu::SelectedSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();
	for (const TTuple<int32, UMVVM_LoadSlot*>& LoadSlot : LoadSlots)
	{
		const int32 SlotIndex = LoadSlot.Key;
		LoadSlot.Value->EnableSelectSlotButton.Broadcast(SlotIndex != Slot);
	}

	SelectedSlot = LoadSlots[Slot];
}

void UMVVM_LoadMenu::DeleteButtonPressed()
{
	check(SelectedSlot);
	AAuraGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);
	SelectedSlot->SlotStatus = Vacant;
	SelectedSlot->EnableSelectSlotButton.Broadcast(true );
	SelectedSlot->InitializeSlot();
	SelectedSlot = nullptr;
}

void UMVVM_LoadMenu::SetNumSlots(int32 NewNumSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumSlots, NewNumSlots);
}

void UMVVM_LoadMenu::LoadData()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	
	for (const TTuple<int32, UMVVM_LoadSlot*>& LoadSlot : LoadSlots)
	{
		ULoadMenuSaveGame* SaveObject = AuraGameMode->GetSaveSlotData(LoadSlot.Value);

		const FString PlayerName = SaveObject->PlayerName;
		const TEnumAsByte<ESaveSlotStatus> SlotStatus = SaveObject->SlotStatus;
		const FString MapName = SaveObject->MapName;

		LoadSlot.Value->SetPlayerName(PlayerName);
		LoadSlot.Value->SlotStatus = SlotStatus;
		LoadSlot.Value->SetMapName(MapName);

		LoadSlot.Value->InitializeSlot();
	}
}
