// Copyright Abdu Inc.


#include "Character/AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystem/Pasive/PassiveNiagaraComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("Burn Debuff Component"));
	BurnDebuffComponent->SetupAttachment(GetRootComponent());
	BurnDebuffComponent->DebuffTag = FAuraGameplayTags::Get().Debuff_Burn;

	StunDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("Stun Debuff Component"));
	StunDebuffComponent->SetupAttachment(GetRootComponent());
	StunDebuffComponent->DebuffTag = FAuraGameplayTags::Get().Debuff_Stun;

	EffectAttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Effect Attach Point"));
	EffectAttachComponent->SetupAttachment(GetRootComponent());

	// Stephen uses Tick to rotate, but some dude used this and works fine
	// https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/41300112#questions/21204486
	EffectAttachComponent->SetUsingAbsoluteRotation(true);
	EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);

	HaloOfProtectionComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("Halo of Protection Component");
	HaloOfProtectionComponent->SetupAttachment(EffectAttachComponent);

	LifeSiphonComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("Life Siphon Component");
	LifeSiphonComponent->SetupAttachment(EffectAttachComponent);

	ManaSiphonComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("Mana Siphon Component");
	ManaSiphonComponent->SetupAttachment(EffectAttachComponent);
	
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


void AAuraCharacterBase::Die(const FVector& InDeathImpulse)
{
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath(InDeathImpulse);
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


int32 AAuraCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}


void AAuraCharacterBase::IncrementMinionCount_Implementation(int32 Amount)
{
	MinionCount += Amount;
}

ECharacterClass AAuraCharacterBase::GetCharacterClass_Implementation() const
{
	return CharacterClass;
}

FOnASCRegistered& AAuraCharacterBase::GetOnASCRegistered()
{
	return OnASCRegistered;
}

FOnDeath* AAuraCharacterBase::GetOnDeath()
{
	return &OnDeath;
}

void AAuraCharacterBase::ApplyKnockback(const FVector& KnockbackImpulse)
{
	LaunchCharacter(KnockbackImpulse, true, true);
}

USkeletalMeshComponent* AAuraCharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}

bool AAuraCharacterBase::IsBeingShocked_Implementation() const
{
	return bIsBeingShocked;
}

void AAuraCharacterBase::SetIsBeingShocked_Implementation(bool InIsBeingShocked)
{
	bIsBeingShocked = InIsBeingShocked;
}


void AAuraCharacterBase::MulticastHandleDeath_Implementation(const FVector& InDeathImpulse)
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	constexpr int WeaponMeshShareOfImpulse = 0.1; // 10%
	Weapon->AddImpulse(
		InDeathImpulse * WeaponMeshShareOfImpulse,
		NAME_None,
		true /* Not considering the mass */
		);
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->AddImpulse(
		InDeathImpulse,
		NAME_None,
		true /* Not considering the mass */
		);

	Dissolve();

	bIsDead = true;

	OnDeath.Broadcast(this);
}


void AAuraCharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAuraCharacterBase, bIsStunned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBeingShocked);
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
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
		UE_LOG(LogTemp, Error, TEXT("CharacterAbilities: Not Authority"));
		return;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent());
	ASC->AddCharacterAbilities(StartupAbilities);
	ASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}


FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon))
	{
		check(Weapon);
		check(!WeaponTipSocketName.IsNone());
		return Weapon->GetSocketLocation(WeaponTipSocketName); 
	}
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_LeftHand))
	{
		check(!LeftHandSocketName.IsNone());
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_RightHand))
	{
		check(!RightHandSocketName.IsNone());
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_Tail))
	{
		check(!TailSocketName.IsNone());
		return GetMesh()->GetSocketLocation(TailSocketName);
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

void AAuraCharacterBase::OnRep_IsStunned(bool OldIsStunned) const
{
	
}


void AAuraCharacterBase::IsStunnedChanged(const FGameplayTag Callbacktag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	const int AAA = bIsStunned ? 0.f : BaseWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
}
