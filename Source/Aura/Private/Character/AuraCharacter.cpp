// Copyright Abdu Inc.


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"


AAuraCharacter::AAuraCharacter()
{
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SprintArm");
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300;

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);
	

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 400, 0);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
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
	// Since Abilities are granted in the server, 
	// and this function works in the server, we are good.
	AddCharacterAbilities();
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


int32 AAuraCharacter::GetPlayerLevel()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	return AuraPlayerState->GetPlayerLevel(); 
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

	InitializeDefaultAttributes();
}


