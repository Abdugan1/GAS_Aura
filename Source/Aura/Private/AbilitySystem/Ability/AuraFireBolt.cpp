// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraFireBolt.h"

#include "AuraGameplayTags.h"


FString UAuraFireBolt::GetDescription(int32 Level)
{
	return GetDescriptionInternal("FIRE BOLT", Level);
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	return GetDescriptionInternal("NEXT LEVEL", Level);
}

FString UAuraFireBolt::GetDescriptionInternal(const FString& Title, int32 Level) const
{
	const int32 AbilityDamage = FMath::RoundHalfToEven(DamageScalableFloat.GetValueAtLevel(Level));
	const int32 FireBoltsAmount = Level;
	const float ManaCost = GetManaCost(Level);
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		"<Title>%s</>\n\n"
		
		"<Small>Level: </><Level>%i</>\n"
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Launches %i bolts of fire, exploding on impact and dealing: </>"
		"<Damage>%i</> <Default>fire damage with a chance to burn</>\n\n"
		), *Title, Level, ManaCost, Cooldown, FireBoltsAmount, AbilityDamage);
}