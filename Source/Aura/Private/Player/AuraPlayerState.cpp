// Copyright Abdu Inc.


#include "Player/AuraPlayerState.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Net/UnrealNetwork.h"


AAuraPlayerState::AAuraPlayerState()
{
	NetUpdateFrequency = 100.f;

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}


UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraPlayerState, Level);
	DOREPLIFETIME(AAuraPlayerState, XP);
}

void AAuraPlayerState::SetXP(int32 NewXP)
{
	XP = NewXP;
	OnXPChanged.Broadcast(NewXP);
}

void AAuraPlayerState::SetLevel(int32 NewLevel)
{
	Level = NewLevel;
	OnLevelChanged.Broadcast(NewLevel);
}

void AAuraPlayerState::AddToXp(int32 AdditionalXP)
{
	SetXP(XP + AdditionalXP);
}

void AAuraPlayerState::AddToLevel(int32 AdditionalLevel)
{
	SetLevel(Level + AdditionalLevel);
}

void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChanged.Broadcast(Level);
}

void AAuraPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChanged.Broadcast(XP);
}
