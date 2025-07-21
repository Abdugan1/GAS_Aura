// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlotSelected);

class UMVVM_LoadSlot;
/**
 * 
 */
UCLASS()
class AURA_API UMVVM_LoadMenu : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:
	void InitializeLoadSlots();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> UMVVM_LoadSlot_Class;

	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 Index) const;
	
	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 Slot, const FString& EnteredName);

	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 Slot);

	UFUNCTION(BlueprintCallable)
	void SelectedSlotButtonPressed(int32 Slot);

	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed();
	
	void SetNumSlots(int32 NewNumSlots);
	int32 GetNumSlots() const {return NumSlots;};

	void LoadData();

	UPROPERTY(BlueprintAssignable)
	FSlotSelected SlotSelected;
	
private:
	UPROPERTY()
	TMap<uint32, UMVVM_LoadSlot*> LoadSlots;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_0;
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_1;
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_2;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> SelectedSlot;
	
	/** NOTE! This is a dummy field notify. It's in beta so it needs at least one field notify bound to something
	 * just to make the whole thing work.
	 */
	/** Field Notifies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess=true))
	int32 NumSlots;
};

