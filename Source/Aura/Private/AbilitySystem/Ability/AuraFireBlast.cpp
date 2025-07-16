// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraFireBlast.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{
	return GetDescriptionInternal("FIRE BLAST", Level);
}

FString UAuraFireBlast::GetNextLevelDescription(int32 Level)
{
	return GetDescriptionInternal("NETX LEVEL", Level);
}

FString UAuraFireBlast::GetDescriptionInternal(const FString& Title, int32 Level) const
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
		
		"<Default>Launches %i fire balls, each coming back and exploding upon return, causing: </>"
		"<Damage>%i</> <Default>fire damage with a chance to burn</>\n\n"
		), *Title, Level, ManaCost, Cooldown, FireBoltsAmount, AbilityDamage);
}
