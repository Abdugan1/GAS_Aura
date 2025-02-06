// Copyright Abdu Inc.


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")));
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}


void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent *TargetACS = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetACS == nullptr)
	{
		return;
	}

	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle EffectContextHandle = TargetACS->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetACS->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContextHandle);
	
	TargetACS->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}
