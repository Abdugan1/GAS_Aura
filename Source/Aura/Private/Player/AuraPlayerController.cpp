// Copyright Abdu Inc.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}


void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();

	AutoRun();
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


void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning)
	{
		return;
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(
			ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(
			LocationOnSpline, ESplineCoordinateSpace::World);

		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = FVector::Dist(LocationOnSpline, CachedDestination);
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
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


void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag Tag)
{
	if (Tag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = CurrentActor != nullptr;
		bAutoRunning = false;
	}
}


void AAuraPlayerController::AbilityInputKeyReleased(FGameplayTag Tag)
{
	// If the button is not LBM || if we weren't hovering something during the press,
	// We call GAS function
	if (!Tag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB) || bTargeting)
	{
		if (GetAuraAbilitySystemComponent() != nullptr)
		{
			GetAuraAbilitySystemComponent()->AbilityInputKeyReleased(Tag);
		}
		return;
	}

	// If it was just a single mouse click, not holding,
	// then find a path to clicked location to auto run
	if (FollowTime <= ShortPressThreshold)
	{
		APawn* ControlledPawn = GetPawn<APawn>();
		if (UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
		{
			Spline->ClearSplinePoints();
			for (const FVector& Point : NavigationPath->PathPoints)
			{
				Spline->AddSplinePoint(Point, ESplineCoordinateSpace::World);
				DrawDebugSphere(GetWorld(), Point, 8, 50, FColor::Green, false, 5.f);
			}

			// Point under cursor may be unreachable. So to avoid infinite running, replace the path's last point
			// as our CachedDestination as we are comparing the distance using CachedDestination
			CachedDestination = NavigationPath->PathPoints[NavigationPath->PathPoints.Num() - 1];
			
			bAutoRunning = true;
		}
	}

	FollowTime = 0.f;
	bTargeting = false; 
}


void AAuraPlayerController::AbilityInputKeyHeld(FGameplayTag Tag)
{
	// If the button is not LBM || if we weren't hovering something during the press,
	// We call GAS function
	if (!Tag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB) || bTargeting)
	{
		if (GetAuraAbilitySystemComponent() != nullptr)
		{
			GetAuraAbilitySystemComponent()->AbilityInputKeyHeld(Tag);
		}
		return;
	}

	FollowTime += GetWorld()->GetDeltaSeconds();

	// Get the location under the cursor and move there
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		CachedDestination = Hit.ImpactPoint;
	}

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// Basically LookAt function
		const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection);
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

