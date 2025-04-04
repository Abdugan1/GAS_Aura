// Copyright Abdu Inc.


#include "Character/AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"


AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	// GE_FireArea is applying the effect twice because it's overlapping both with the capsule and the mesh
	// Turning overlap events on the capsule does the trick
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}


void AAuraCharacterBase::Die()
{
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath();
}


void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Dissolve();
}


void AAuraCharacterBase::InitAbilityActorInfo()
{
}


void AAuraCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);

	// This, I think, is a boilerplate. A context is whatever the owner of the GAS has at the moment.
	// You can store some necessary(which I don't know what kind of :D, for now, it's just the instigator) things into it. 
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	// This is kind of a instigator.
	ContextHandle.AddSourceObject(this);

	// The actual creation of an effect. Right now, storing 
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}


void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	// The order matters! Primary -> Secondary -> Vital
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}


void AAuraCharacterBase::AddCharacterAbilities() const
{
	if (!HasAuthority())
	{
		return;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent());
	ASC->AddCharacterAbilities(StartupAbilities);	
}


FVector AAuraCharacterBase::GetCombatSocketLocation()
{
	check(Weapon);
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}


void AAuraCharacterBase::Dissolve()
{
	// Dissolving is made setting the material of the mesh to the dissolving one.
	// Basically, we are swapping the default mesh with a special one.
	if (IsValid(MeshDissolveMaterialInstanceClass))
	{
		UMaterialInstanceDynamic* DissolveMaterialInstance = UMaterialInstanceDynamic::Create(MeshDissolveMaterialInstanceClass, this);
		GetMesh()->SetMaterial(0, DissolveMaterialInstance);

		StartMeshDissolveTimeline(DissolveMaterialInstance);
	}

	if (IsValid(WeaponDissolveMaterialInstanceClass))
	{
		UMaterialInstanceDynamic* DissolveMaterialInstance = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstanceClass, this);
		Weapon->SetMaterial(0, DissolveMaterialInstance);

		StartWeaponDissolveTimeline(DissolveMaterialInstance);
	}
}
