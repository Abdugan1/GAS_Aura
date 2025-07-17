// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraFireBlast.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireBall.h"

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

TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> FireBalls;
	
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumFireBalls);

	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());
		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass,
			SpawnTransform,
			GetAvatarActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);

		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();
		
		FireBalls.Add(FireBall);

		FireBall->FinishSpawning(SpawnTransform);
	}
	
	return FireBalls;
}
