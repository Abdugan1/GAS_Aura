// Copyright Abdu Inc.


#include "AbilitySystem/Ability/AuraSummonAbility.h"

#include "Kismet/KismetSystemLibrary.h"


TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	TArray<FVector> SpawnLocations;

	const FVector ForwardVector = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector AvatarLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
	
	const float DeltaSpread = SpawnSpreadAngle / NumMinions;

	// const FVector RightOfSpread = ForwardVector.RotateAngleAxis(SpawnSpreadAngle / 2.f, FVector::UpVector);
	const FVector LeftOfSpread = ForwardVector.RotateAngleAxis(-SpawnSpreadAngle / 2.f, FVector::UpVector);
	for (int32 i = 0; i < NumMinions; i++)
	{
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
		FVector ChosenSpawnLocation = AvatarLocation + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		/**
		 * Since we're not using an EQS, we must make do with linetracing to prevent spawning inside walls
		 */
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector{0.f, 0.f, 400.f}, ChosenSpawnLocation - FVector{0.f, 0.f, 400.f}, ECC_Visibility);
		if (Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
		}
		
		SpawnLocations.Add(ChosenSpawnLocation);

		/** Debug drawings */
		// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), ChosenSpawnLocation + FVector{0.f, 0.f, 400.f}, ChosenSpawnLocation - FVector{0.f, 0.f, 400.f}, 4.f, FColor::Black, 3);
		// DrawDebugSphere(GetWorld(), ChosenSpawnLocation, 18.f, 12, FColor::Cyan, false, 3.f);
		// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), AvatarLocation, AvatarLocation + Direction * MaxSpawnDistance, 4.f, FColor::Green, 3);
		// DrawDebugSphere(GetWorld(), AvatarLocation + Direction * MaxSpawnDistance, 5.f, 12, FColor::Red, false, 3.f);
		// DrawDebugSphere(GetWorld(), AvatarLocation + Direction * MinSpawnDistance, 5.f, 12, FColor::Red, false, 3.f);
	}
	
	return SpawnLocations;
}


TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];	
}
