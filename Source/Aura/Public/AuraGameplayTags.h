// Copyright Abdu Inc.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 *
 * Singleton containing native Gameplay Tags
 */
struct FAuraGameplayTags
{
public:
	static const FAuraGameplayTags& Get();
	static void InitializeNativeGameplayTags();

	//** Primary Attributes */
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;
	//** end Primary Attributes */
	
	// ** Secondary Attributes */
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;
	// ** end Secondary Attributes */

	/** Meta Attributes */
	FGameplayTag Attributes_Meta_IncomingXP;
	/** end Meta Attributes */
	
	/** Resistances */
	/** Look the damage types comment before adding anything here! */
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;
	/** end Resistances */
	
	//** Input */
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;
	//** end Input */

	//** Damage */
	/**
	 * If you want to add a damage type, do these:
	 * 1. Add a correspond Gameplay Tag
	 * 2. Add a corresponding RESISTANCE Gameplay Tag for the DamageType Gameplay Tag
	 * 3. Initialize both GameplayTags InitializeNativeGameplayTags
	 * 4. Add both GameplayTags to DamageTypesToResistances
	 */
	FGameplayTag Damage;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Physical;

	// DamageType Gameplay Tag -> Resistance GameplayTag to that Damage Type  
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	//** end Damage */

	/** Debuffs */
	/**
	 * If you want to add a debug type, do these:
	 * 1. Add a correspond Gameplay Tag
	 * 2. Make sure it has a corresponding DamageType
	 * 3. Add both GameplayTags to DamageTypesToDebuffs
	 */
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Arcane;
	FGameplayTag Debuff_Physical;

	FGameplayTag Debuff_Parameter_Chance;
	FGameplayTag Debuff_Parameter_Damage;
	FGameplayTag Debuff_Parameter_Frequency;
	FGameplayTag Debuff_Parameter_Duration;
	
	TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;
	/** end Debuffs */
	
	/** Abilities */
	FGameplayTag Abilities_None;
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;
	
	FGameplayTag Abilities_HitReact;

	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;

	FGameplayTag Abilities_Type_Offensive;
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_None;
	
	FGameplayTag Abilities_Fire_FireBolt;
	FGameplayTag Abilities_Lightning_Electrocute;
	/** end Abilities*/

	/** Cooldown */
	FGameplayTag Cooldown_Fire_FireBolt;
	FGameplayTag Cooldown_Lightning_Electrocute;
	/** end Cooldown */
	
	/** CombatSocket */
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_Tail;
	/** end CombatSocket*/

	/** Montage Attacks*/
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;
	/** end Montage Attacks*/
	
	//** Effects */
	FGameplayTag Effects_HitReact;
	//** end Effects */

	/** Player */
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;
	/** end Player */
	
private:
	static FAuraGameplayTags GameplayTags;

	bool bInitialized = false;
};