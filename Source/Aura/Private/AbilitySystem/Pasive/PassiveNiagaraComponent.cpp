// Copyright Abdu Inc.


#include "AbilitySystem/Pasive/PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Interaction/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	UAuraAbilitySystemComponent* AuraACS = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()));
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());
	if (AuraACS)
	{
		AuraACS->ActivatePassiveEffectDelegate.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
	}
	else if (CombatInterface)
	{
		CombatInterface->GetOnASCRegistered().AddWeakLambda(this,
			[this](UAbilitySystemComponent* InAbilitySystemComponent)
			{
				UAuraAbilitySystemComponent* AuraACS = Cast<UAuraAbilitySystemComponent>(InAbilitySystemComponent);
				AuraACS->ActivatePassiveEffectDelegate.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
			});
	}
}

void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		if (bActivate && !IsActive())
		{
			Activate();
		}
		else
		{
			Deactivate();
		}
	}
}
