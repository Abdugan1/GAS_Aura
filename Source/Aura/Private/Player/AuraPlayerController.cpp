// Copyright Abdu Inc.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Character/AuraCharacter.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Player/AuraPlayerState.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}


void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}


void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	// TODO: Don't know why we need to do this. It's because of multiplayer. Need further investigations.
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Type::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);

	SetInputMode(InputModeData);
}


void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputKeyReleased, &ThisClass::AbilityInputKeyHeld);
}


void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}

}


void AAuraPlayerController::CursorTrace()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	if (!Hit.bBlockingHit)
	{
		return;
	}

	LastActor = CurrentActor;
	CurrentActor = Hit.GetActor();

	/*
	 * Line trance from the cursor. There are several scenarios:
	 * A. LastActor is null && CurrentActor is null
	 *  - Do nothing.
	 * B. LastActor is null && CurrentActor is valid
	 *  - Highlight CurrentActor
	 * C. LastActor is valid && CurrentActor is null
	 *  - UnHighlight LastActor
	 * D. Both actors are valid, but LastActor != CurrentActor
	 *  - UnHighlight LastActor, and Highlight CurrentActor
	 * E. Both actors are valid, and are the same actor
	 *  - Do nothing.
	 */
	
	if (LastActor == nullptr)
	{
		if (CurrentActor != nullptr)
		{
			// Case B
			CurrentActor->HighlightActor();
		}
		else
		{
			// Case A -- do nothing.
		}
	}
	else // LastActor is valid
	{
		if (CurrentActor == nullptr)
		{
			// Case C
			LastActor->UnHighlightActor();
		}
		else // both actors are valid
		{
			if (LastActor != CurrentActor)
			{
				// Case D
				LastActor->UnHighlightActor();
				CurrentActor->HighlightActor();
			}
			else
			{
				// Case E -- do nothing.
			}
		}
	}
}


void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag Tag)
{
	// GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *Tag.ToString());
}


void AAuraPlayerController::AbilityInputKeyReleased(FGameplayTag Tag)
{
	if (GetAuraAbilitySystemComponent() != nullptr)
	{
		GetAuraAbilitySystemComponent()->AbilityInputKeyReleased(Tag);
	}
}


void AAuraPlayerController::AbilityInputKeyHeld(FGameplayTag Tag)
{
	if (GetAuraAbilitySystemComponent() != nullptr)
	{
		GetAuraAbilitySystemComponent()->AbilityInputKeyHeld(Tag);
	}
}


UAuraAbilitySystemComponent* AAuraPlayerController::GetAuraAbilitySystemComponent()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

