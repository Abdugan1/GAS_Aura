// Copyright Abdu Inc.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Game/AuraGameModeBase.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"


UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	// TODO: PlayerController index is 0. Multiplayer issues?
	APlayerController *PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	check(PlayerController);
	AAuraHUD* AuraHUD = Cast<AAuraHUD>(PlayerController->GetHUD());
	check(AuraHUD);
	
	const FWidgetControllerParams WidgetControllerParams = GetWidgetControllerParams(WorldContextObject, PlayerController);
	return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);
}


UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	// TODO: PlayerController index is 0. Multiplayer issues?
	APlayerController *PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	check(PlayerController);
	AAuraHUD* AuraHUD = Cast<AAuraHUD>(PlayerController->GetHUD());
	check(AuraHUD);
	
	const FWidgetControllerParams WidgetControllerParams = GetWidgetControllerParams(WorldContextObject, PlayerController);
	return AuraHUD->GetAttributeMenuController(WidgetControllerParams);
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(
	const UObject* WorldContextObject)
{
	// TODO: PlayerController index is 0. Multiplayer issues?
	APlayerController *PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	check(PlayerController);
	AAuraHUD* AuraHUD = Cast<AAuraHUD>(PlayerController->GetHUD());
	check(AuraHUD);
	
	const FWidgetControllerParams WidgetControllerParams = GetWidgetControllerParams(WorldContextObject, PlayerController);
	return AuraHUD->GetSpellMenuWidgetController(WidgetControllerParams);
}


void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject,
                                                            ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* AbilitySystemComponent)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

	if (CharacterClassInfo == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterClassInfo is null in InitializeDefaultAttributes(). Called on a Client?"));
		return;
	}
	
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	
	ApplyEffectToASC(AbilitySystemComponent, ClassDefaultInfo.PrimaryAttributes, Level);
	ApplyEffectToASC(AbilitySystemComponent, CharacterClassInfo->SecondaryAttributes, Level);
	ApplyEffectToASC(AbilitySystemComponent, CharacterClassInfo->VitalAttributes, Level);
	
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject,
	UAbilitySystemComponent* AbilitySystemComponent, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterClassInfo is null in GiveStartupAbilities(). Called on a Client?"));
		return;
	}
	
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1.f);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (auto AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (AbilitySystemComponent->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(AbilitySystemComponent->GetAvatarActor()));
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}


int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
	ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr)
	{
		UE_LOG(LogAura, Error, TEXT("CharacterClassInfo is null in GetXPRewardForClassAndLevel(). Called on a Client?"));
		return 0;
	}
	const FCharacterClassDefaultInfo Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Result;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);

	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Result.Add(Direction.Rotation());
		}
	}
	else
	{
		Result.Add(Forward.Rotation());
	}

	return Result;
}

TArray<FVector> UAuraAbilitySystemLibrary::EvenlySpacedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Result;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);

	if (NumVectors > 1)
	{
		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Result.Add(Direction);
		}
	}
	else
	{
		Result.Add(Forward);
	}

	return Result;
}


UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraGameMode is null. Called on a Client? GameModes cannot be on Clients."));
		return nullptr;
	}

	return AuraGameMode->CharacterClassInfo;
}


UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraGameMode is null. Called on a Client? GameModes cannot be on Clients."));
		return nullptr;
	}

	return AuraGameMode->AbilityInfo;
}

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& Params)
{
	const AActor* SourceAvatarActor = Params.SourceAbilitySystemComponent->GetAvatarActor();

	if (!SourceAvatarActor->HasAuthority())
	{
		UE_LOG(LogAura, Warning, TEXT("Attempting to apply damage on Clients! Skipping"));
		return FGameplayEffectContextHandle();
	}
	
	FGameplayEffectContextHandle EffectContextHandle = Params.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContextHandle, Params.DeathImpulse);
	SetKnockbackImpulse(EffectContextHandle, Params.KnockbackImpulse);
	
 	const FGameplayEffectSpecHandle EffectSpecHandle = Params.SourceAbilitySystemComponent->MakeOutgoingSpec(Params.DamageGameplayEffectClass, Params.AbilityLevel, EffectContextHandle);

	// Base Damage
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, Params.DamageTypeTag, Params.BaseDamage);

	// Debuff Params
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FAuraGameplayTags::Get().Debuff_Parameter_Chance, Params.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FAuraGameplayTags::Get().Debuff_Parameter_Damage, Params.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FAuraGameplayTags::Get().Debuff_Parameter_Duration, Params.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, FAuraGameplayTags::Get().Debuff_Parameter_Frequency, Params.DebuffFrequency);
	
	
	Params.SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data, Params.TargetAbiltySystemComponent);

	return EffectContextHandle;
}

bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->IsSuccessfulDebuff();
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// I think no use to check to see if it's a valid pointer, since static_cast doesn't do it -- not dynamic_cast
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->IsBlockedHit();
}


bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// I think no use to check to see if it's a valid pointer, since static_cast doesn't do it -- not dynamic_cast
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->IsCriticalHit();
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->GetDebuffDamage();
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->GetDebuffDuration();
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->GetDebuffFrequency();
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageTypeTag(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->GetDamageTypeTag();
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->GetDeathImpulse();
}

FVector UAuraAbilitySystemLibrary::GetKnockbackImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	return AuraEffectContext->GetKnockbackImpulse();
}

void UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
                                                      bool IsSuccessful)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetIsSuccessfulDebuff(IsSuccessful);
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
}


void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle,
	float InDamage)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetDebuffDamage(InDamage);
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle,
	float InDuration)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetDebuffDuration(InDuration);
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle,
	float InFrequency)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetDebuffFrequency(InFrequency);
}

void UAuraAbilitySystemLibrary::SetDamageTypeTag(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetDamageTypeTag(InDamageType);
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InImpulse)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetDeathImpulse(InImpulse);
}

void UAuraAbilitySystemLibrary::SetKnockbackImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InImpulse)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	AuraEffectContext->SetKnockbackImpulse(InImpulse);
}

void UAuraAbilitySystemLibrary::GetLiveActorsWithinRadius(const UObject* WorldContextObject,
                                                          TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
                                                          const FVector& SphereOrigin)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> OverlapResults;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(OverlapResults,
			SphereOrigin,
			FQuat::Identity,
			FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
			FCollisionShape::MakeSphere(Radius),
			Params
			);
		for (FOverlapResult& OverlapResult : OverlapResults)
		{
			if (OverlapResult.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(OverlapResult.GetActor()))
			{
				OutOverlappingActors.AddUnique(OverlapResult.GetActor());
			}
		}
	}
}

void UAuraAbilitySystemLibrary::GetClosestTargets(int32 MaxNumTargets, const TArray<AActor*>& Actors,
	TArray<AActor*>& OutTargets, const FVector& Origin)
{
	if (Actors.Num() <= MaxNumTargets)
	{
		OutTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while (NumTargetsFound < MaxNumTargets)
	{
		if (ActorsToCheck.Num() == 0)
		{
			break;
		}
		
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor = nullptr;
		for (AActor* Actor : ActorsToCheck)
		{
			const double Distance = (Actor->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = Actor;
			}
		}
		ActorsToCheck.Remove(ClosestActor);
		OutTargets.AddUnique(ClosestActor);

		NumTargetsFound++;
	}
}


bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bFirstIsPlayer = FirstActor->ActorHasTag(FName("Player"));
	const bool bSecondIsPlayer = SecondActor->ActorHasTag(FName("Player"));

	const bool bFirstIsEnemy = FirstActor->ActorHasTag(FName("Enemy"));
	const bool bSecondIsEnemy = SecondActor->ActorHasTag(FName("Enemy"));

	const bool bBothArePlayers = bFirstIsPlayer && bSecondIsPlayer;
	const bool bBothAreEnemies = bFirstIsEnemy && bSecondIsEnemy;

	const bool bFriends = bBothAreEnemies || bBothArePlayers;

	return !bFriends;
}


void UAuraAbilitySystemLibrary::ApplyEffectToASC(UAbilitySystemComponent* AbilitySystemComponent,
                                                 TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level)
{
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(AbilitySystemComponent->GetAvatarActor());
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}


FWidgetControllerParams UAuraAbilitySystemLibrary::GetWidgetControllerParams(const UObject* WorldContextObject, APlayerController *PlayerController)
{
	AAuraPlayerState *PlayerState = PlayerController->GetPlayerState<AAuraPlayerState>();
	UAbilitySystemComponent *AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
	UAttributeSet *AttributeSet = PlayerState->GetAttributeSet();
	
	return FWidgetControllerParams (PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);
}
