// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()
public:
	AAuraCharacter();
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UCameraComponent* CameraComp;
	
};
