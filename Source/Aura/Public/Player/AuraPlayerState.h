// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class ULevelUpInfo;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/);

class UAttributeSet;
/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	/** public FUNCTIONS */
public:
	AAuraPlayerState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }
	
	void SetXP(int32 NewXP);
	void SetLevel(int32 NewLevel);
	void SetAttributePoints(int32 NewAttributePoints);
	void SetSpellPoints(int32 NewSpellPoints);
	
	
	void AddToXp(int32 AdditionalXP);
	void AddToLevel(int32 AdditionalLevel);
	void AddToAttributePoints(int32 AdditionalAttributePoints);
	void AddToSpellPoints(int32 AdditionalSpellPoints);

	/** public DELEGATES */
public: 
	FOnPlayerStatChanged OnXPChanged;
	FOnPlayerStatChanged OnLevelChanged;
	FOnPlayerStatChanged OnAttributePointsChanged;
	FOnPlayerStatChanged OnSpellPointsChanged;

	/** private FUNCTIONS FOR REPLICATION */
private:
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UFUNCTION()
	void OnRep_XP(int32 OldXP);

	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSpellPoints);

	/** public VARIABLES */
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;
	
	/** protected VARIABLES */
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/** private VARIABLES */
private:
	UPROPERTY(VisibleAnywhere, Replicated=OnRep_Level)
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, Replicated=OnRep_XP)
	int32 XP = 0;

	UPROPERTY(VisibleAnywhere, Replicated=OnRep_AttributePoints)
	int32 AttributePoints = 0;

	UPROPERTY(VisibleAnywhere, Replicated=OnRep_SpellPoints)
	int32 SpellPoints = 0;
};

