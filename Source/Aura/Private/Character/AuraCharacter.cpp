// Copyright Abdu Inc.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "NiagaraComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Game/AuraGameInstance.h"
#include "Game/AuraGameModeBase.h"
#include "Game/LoadMenuSaveGame.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"

AAuraCharacter::AAuraCharacter()
{
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SprintArm");
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300;

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);

	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagara");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 400, 0);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CharacterClass = ECharacterClass::Elementalist;
}


/*
 * Init AbilitySystemComponent. Works SERVER ONLY. ASC is in the PlayerState.
 * DO NOT forget to set the ASC and AS variables that are in the base class.
 * To get it to work in the CLIENT, we need to do it in OnRep_PlayerState
 */
void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();

	LoadProgress();
	
	/** TODO: Load in Abilities from disk */
	// Since Abilities are granted in the server, 
	// and this function works in the server, we are good.
	// AddCharacterAbilities(); REMOVED. WENT TO LoadProgress
}


/*
 * Init AbilitySystemComponent. Works CLIENT ONLY. ASC is in the PlayerState.
 * DO NOT forget to set the ASC and AS variables that are in the base class.
 * To get it to work in the SERVER, we need to do it in PossessedBy
 */
void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}


void AAuraCharacter::LoadProgress()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	ULoadMenuSaveGame* SaveData = AuraGameMode->RetrieveInGameSaveData();
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();


	if (SaveData->bFirstTimeLoadIn)
	{
		InitializeDefaultAttributes();
		/** TODO: Load in Abilities from disk */
		AddCharacterAbilities();
	}
	else
	{
		AuraPlayerState->SetLevel(SaveData->PlayerLevel);
		AuraPlayerState->SetXP(SaveData->XP);
		AuraPlayerState->SetAttributePoints(SaveData->AttributePoints);
		AuraPlayerState->SetSpellPoints(SaveData->SpellPoints);
		UAuraAbilitySystemLibrary::InitializeDefaultAttributesFromSavedData(this, GetAbilitySystemComponent(), SaveData);
		ApplyEffectToSelf(DefaultSecondaryAttributes, AuraPlayerState->GetPlayerLevel());
		ApplyEffectToSelf(DefaultVitalAttributes, AuraPlayerState->GetPlayerLevel());
	}
}


int32 AAuraCharacter::GetPlayerLevel_Implementation()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	return AuraPlayerState->GetPlayerLevel(); 
}


void AAuraCharacter::AddToXp_Implementation(int32 InAdditionalXP)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToXp(InAdditionalXP);
}

void AAuraCharacter::AddToPlayerLevel_Implementation(int32 InAdditionalPlayerLevel)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToLevel(InAdditionalPlayerLevel);

	// This MAY be a more reliable approach. Since Broadcasts are usually for cosmetics.
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(GetAbilitySystemComponent());
	AuraASC->UpdateAbilityStatuses(AuraPlayerState->GetPlayerLevel());
}

void AAuraCharacter::AddToAttributePoints_Implementation(int32 InAdditionalAttributePoints)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToAttributePoints(InAdditionalAttributePoints);
}

void AAuraCharacter::AddToSpellPoints_Implementation(int32 InAdditionalSpellPoints)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddToSpellPoints(InAdditionalSpellPoints);
}

int32 AAuraCharacter::GetXP_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetXP();
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInfos[Level].AttributePointReward;
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->LevelUpInfos[Level].SpellPointReward;
}

int32 AAuraCharacter::GetAttributePoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetAttributePoints();
}

int32 AAuraCharacter::GetSpellPoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetSpellPoints();
}


void AAuraCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

int32 AAuraCharacter::FindLevelForXp_Implementation(int32 InXP)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

void AAuraCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	AAuraPlayerController* AuraPlayerController = GetController<AAuraPlayerController>();
	check(AuraPlayerController);
	AuraPlayerController->ShowMagicCircle(DecalMaterial);
	AuraPlayerController->bShowMouseCursor = false;
}

void AAuraCharacter::HideMagicCircle_Implementation()
{
	AAuraPlayerController* AuraPlayerController = GetController<AAuraPlayerController>();
	check(AuraPlayerController);
	AuraPlayerController->HideMagicCircle();
	AuraPlayerController->bShowMouseCursor = true;
}

void AAuraCharacter::SaveProgress_Implementation(const FName& PlayerStartTag)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	ULoadMenuSaveGame* SaveData = AuraGameMode->RetrieveInGameSaveData();
	SaveData->PlayerStartTag = PlayerStartTag;

	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	SaveData->PlayerLevel = AuraPlayerState->GetPlayerLevel();
	SaveData->XP = AuraPlayerState->GetXP();
	SaveData->AttributePoints = AuraPlayerState->GetAttributePoints();
	SaveData->SpellPoints = AuraPlayerState->GetSpellPoints();

	UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(GetAttributeSet());
	SaveData->Strength = AuraAttributeSet->GetStrength();
	SaveData->Intelligence = AuraAttributeSet->GetIntelligence();
	SaveData->Resilience = AuraAttributeSet->GetResilience();
	SaveData->Vigor = AuraAttributeSet->GetVigor();

	SaveData->bFirstTimeLoadIn = false;
	
	AuraGameMode->SaveInGameProgressData(SaveData);
}


/**
 * This is basically called in both the client and sever.
 * No need to worry about PlayerState and PlayerController whether we received it.
 */
void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	// Ability System Comp and Attribute Set is in the PlayerState.
	// So we need to set our pointers to it. Otherwise, it always returns nullptr.
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);

	OnASCRegistered.Broadcast(AbilitySystemComponent);

	// We must call this to broadcast effects applied
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// Basically we are setting up the UI, i.e., the HUD.
	// Clients do NOT have a PlayerController for all Pawns. Only the client's pawn has a valid controller.
	if (AAuraPlayerController* AuraPlayerController =  Cast<AAuraPlayerController>(GetController()))
	{
		if (AAuraHUD *AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	// InitializeDefaultAttributes(); REMOVED
	
	AbilitySystemComponent->RegisterGameplayTagEvent(
		FAuraGameplayTags::Get().Debuff_Stun,
		EGameplayTagEventType::NewOrRemoved)
	.AddUObject(this, &AAuraCharacter::IsStunnedChanged);

	// If you're looking where Abilities are given, check PossessedBy. Only Server must give abilities
}

void AAuraCharacter::OnRep_IsStunned(bool OldIsStunned) const
{
	// UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	FGameplayTagContainer BlockedTags;
	BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_CursorTrace);
	BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputHeld);
	BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputPressed);
	BlockedTags.AddTag(FAuraGameplayTags::Get().Player_Block_InputReleased);

	if (bIsStunned)
	{
		AbilitySystemComponent->AddLooseGameplayTags(BlockedTags);
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTags(BlockedTags);
	}
}


void AAuraCharacter::MulticastLevelUpParticles_Implementation()
{
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = CameraComp->GetComponentLocation();
		const FVector NiagaraLocation = LevelUpNiagaraComponent->GetComponentLocation();

		const FRotator ToCameraRotation = (CameraLocation - NiagaraLocation).Rotation();

		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		
		LevelUpNiagaraComponent->Activate(true);
	}
}


