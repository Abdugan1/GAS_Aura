// Copyright Abdu Inc.


#include "Character/AuraEnemy.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AI/AuraAIController.h"
#include "Aura/Aura.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UObject/ReferenceChainSearch.h"


AAuraEnemy::AAuraEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Enemies are AI-PlayerController entities. No PlayerState. So we can just safely assign these right here.
	// But we need to init it in the BeginPlay
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>("HealthBarWidget");
	HealthBarWidget->SetupAttachment(RootComponent);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	HitReactingKey = "HitReacting";
	RangedAttackerKey = "RangedAttacker";
}


void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority())
	{
		return;
	}
	
	AuraAIController = Cast<AAuraAIController>(NewController);

	AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	AuraAIController->RunBehaviorTree(BehaviorTree);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(HitReactingKey, false);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(RangedAttackerKey, CharacterClass != ECharacterClass::Warrior);
}


void AAuraEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	Weapon->SetRenderCustomDepth(true);

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}


void AAuraEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}


void AAuraEnemy::SetCombatTarget_Implementation(AActor* NewCombatActor)
{
	CombatTarget = NewCombatActor;
}


AActor* AAuraEnemy::GetCombatTarget_Implementation()
{
	return CombatTarget;
}


int32 AAuraEnemy::GetPlayerLevel()
{
	return Level;
}


void AAuraEnemy::Die()
{
	SetLifeSpan(LifeSpan);
	Super::Die();
}


void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	
	InitAbilityActorInfo();

	AbilitySystemComponent->RegisterGameplayTagEvent(
		FAuraGameplayTags::Get().Effects_HitReact,
		EGameplayTagEventType::NewOrRemoved)
	.AddUObject(this, &AAuraEnemy::HitReactTagChanced);

	if (HasAuthority())
	{
		UAuraAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
	
	InitUi();
}


void AAuraEnemy::HitReactTagChanced(const FGameplayTag Callbacktag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;

	if (HasAuthority())
	{
		// AI controller exists only on the Server
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(HitReactingKey, bHitReacting);
	}
}


void AAuraEnemy::InitAbilityActorInfo()
{
	// Enemies are a AI-PlayerController entities. No PlayerState. The owner and avatar are the same thing.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// We must call this to broadcast effects applied
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}
}


void AAuraEnemy::InitializeDefaultAttributes() const
{
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}


void AAuraEnemy::InitUi()
{
	const UAuraAttributeSet *AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
	
	// We care only about enemy's health attribute.
	// For simplicity's sake, the AuraEnemy itself will be the WidgetController
	UAuraUserWidget* AuraWidget = CastChecked<UAuraUserWidget>(HealthBarWidget->GetWidget());
	AuraWidget->SetWidgetController(this);

	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
}
