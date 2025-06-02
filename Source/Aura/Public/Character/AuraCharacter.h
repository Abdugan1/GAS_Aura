// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
public:
	AAuraCharacter();

	/** Init AbilitySystem: Read more in the implementations **/
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	/** end Init AbilitySystem **/

	/** Combat Interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	/** end Combat Interface */

	/** Player Interface */
	virtual void AddToXp_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	/** end Player Interface */
	
protected:
	virtual void InitAbilityActorInfo() override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UCameraComponent* CameraComp;
	
};
