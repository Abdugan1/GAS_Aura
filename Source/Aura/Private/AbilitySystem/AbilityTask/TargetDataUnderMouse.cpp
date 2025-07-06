// Copyright Abdu Inc.


#include "AbilitySystem/AbilityTask/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "ProfilingDebugging/CookStats.h"


UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}


void UTargetDataUnderMouse::Activate()
{
	// If we are locally controlled , we SEND the mouse cursor data
	// Yes, when we are on the server, there's one 'hanging' target data. No big deal
	if (Ability->GetCurrentActorInfo()->IsLocallyControlled())
	{
		SendMouseCursorData();
	}
	else
	{
		// We are on the server, we must RECEIVE the mouse cursor data

		// This is boilerplate. I hope it is...
		FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		FPredictionKey PredictionKey = GetActivationPredictionKey();

		// Binding the callback when it TargetData received
		AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, PredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);

		// We could or couldn't have received it. If we didn't, then we just wait
		const bool bAlreadyReplicated = AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, PredictionKey);
		if (!bAlreadyReplicated)
		{
			// Server hasn't received the data, just wait until it arrives.
			SetWaitingOnRemotePlayerData();
		}
	}
}


void UTargetDataUnderMouse::SendMouseCursorData()
{
	// Prediction thingies are all boilerplate. I hope...
	FScopedPredictionWindow ScopedPredictionWindow(AbilitySystemComponent.Get());
	
	FHitResult Hit;
	// Get location under cursor. It's done using the PlayerController.
	Ability->GetCurrentActorInfo()->PlayerController->GetHitResultUnderCursor(ECC_Target, false, Hit);

	// UE's ready-to-use structure. No memory leakage is here. It's necessary to use 'new' for some reason I don't know.
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = Hit;

	// This is like a container. Maybe...
	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(Data);

	// Don't know what this is for
	FGameplayTag ApplicationTag;
	
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		ApplicationTag,
		AbilitySystemComponent->ScopedPredictionKey
		);

	/** This should be called prior to broadcasting delegates back into the ability graph. This makes sure the ability is still active.  */
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}


void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	FGameplayTag ActivationTag)
{
	// This line clears all TargetData sent to us.
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
