// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "AuraEnemy.generated.h"

class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewValue);

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
public:
	AAuraEnemy();

	/* Enemy interface */
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	/* end Enemy interface */

	/* Combat Interface */
	virtual int32 GetPlayerLevel() override;
	/* end Combat Interface*/

	UFUNCTION()
	void HitReactTagChanced(const FGameplayTag CallbackTag, int32 NewCount);
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;
	
protected:
	virtual void BeginPlay() override;

	/* AuraCharacterBase interface */
	virtual void InitAbilityActorInfo() override;

	virtual void InitializeDefaultAttributes() const override;
	/* end AuraCharacterBase interface */
	
private:
	void InitUi();

public:
	UPROPERTY(BlueprintReadOnly, Category= "Combat")
	bool bHitReacting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* HealthBarWidget;
};
