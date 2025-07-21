// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadMenuSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 
 */
UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	void InitializeSlot();

	void SetPlayerName(FString NewPlayerName);
	void SetLoadSlotName(FString NewLoadSlotName);
	
	FString GetPlayerName() const {return PlayerName;};
	FString GetLoadSlotName() const {return LoadSlotName;};

public:
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;

	UPROPERTY()
	int32 SlotIndex = -1;
	
private:
	/** Field Notifies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess=true))
	FString PlayerName;

	/** NOTE! This is a dummy field notify. It's in beta so it needs at least one field notify bound to something
	* just to make the whole thing work.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess=true))
	FString LoadSlotName;
};
