// Copyright Abdu Inc.


#include "Character/AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"


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

	// Magic Trick to use on Dedicated Servers
	// https://forums.unrealengine.com/t/run-on-dedicated-server-cause-problems-with-socket-location/361012
	// https://forums.unrealengine.com/t/cost-of-always-tick-pose-and-refresh-bones-on-dedicated-server/123973/2
	// https://discordapp.com/channels/807733033192390676/1089687554070155364/1178232997900714054
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
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


bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bIsDead;
}


AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}


TArray<FTaggedMontage> AAuraCharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontages;
}


UNiagaraSystem* AAuraCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage AAuraCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for (const auto& TaggedMontage : AttackMontages)
	{
		if (TaggedMontage.MontageTag == MontageTag)
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}


void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Dissolve();

	bIsDead = true;
}


void AAuraCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// if (IsValid(Weapon))
	// {
	// 	auto Location = Execute_GetCombatSocketLocation(this, FAuraGameplayTags::Get().CombatSocket_Weapon);
	// 	DrawDebugSphere(GetWorld(), Location, 20, 32, FColor::Red);
	// }
	// if (!LeftHandSocketName.IsNone())
	// {
	// 	auto Location = Execute_GetCombatSocketLocation(this, FAuraGameplayTags::Get().CombatSocket_LeftHand);
	// 	DrawDebugSphere(GetWorld(), Location, 20, 32, FColor::Yellow);
	// }
	// if (!RightHandSocketName.IsNone())
	// {
	// 	auto Location = Execute_GetCombatSocketLocation(this, FAuraGameplayTags::Get().CombatSocket_RightHand);
	// 	DrawDebugSphere(GetWorld(), Location, 20, 32, FColor::Green);
	// }
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


FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon))
	{
		check(Weapon);
		check(WeaponTipSocketName.IsValid());
		return Weapon->GetSocketLocation(WeaponTipSocketName); 
	}
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_LeftHand))
	{
		check(LeftHandSocketName.IsValid());
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_RightHand))
	{
		check(RightHandSocketName.IsValid());
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}

	UE_LOG(LogTemp, Warning, TEXT("GetCombatSocketLocation reached the end without any matches! Did you forget to set MontageTag?"));
	return FVector::ZeroVector;
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
