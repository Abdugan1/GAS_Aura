// Copyright Abdu Inc.


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include <rapidjson/schema.h>
#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Chaos/PBDSuspensionConstraintData.h"
#include "Interaction/CombatInterface.h"


struct AuraDamageStatics
{
	/**
	 * How to add an Attribute?
	 * 1. Declare it
	 * 2. Define it
	 * 3. Add it to the TagsToCaptureDefs map
	 * 4. Add it to RelevantAttributesToCapture
	 */
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistanceFire)
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistanceLightning)
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistanceArcane)
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistancePhysical)

	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistanceFire, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistanceLightning, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistanceArcane, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistancePhysical, Target, false);

		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, CriticalHitChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, CriticalHitDamageDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, CriticalHitResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, ResistanceFireDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, ResistanceLightningDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, ResistanceArcaneDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, ResistancePhysicalDef);
	}
};


static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}


UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResistanceFireDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResistanceLightningDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResistanceArcaneDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResistancePhysicalDef);
}


void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& EffectSpec, FAggregatorEvaluateParameters EvaluateParams) const
{
	for (const auto& Pair : FAuraGameplayTags::Get().DamageTypesToDebuffs)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag DebuffTypeTag = Pair.Value;
		const float TypeDamage = EffectSpec.GetSetByCallerMagnitude(DamageTypeTag, false, -1.f);
		if (TypeDamage > -1.f)
		{
			// Determine if there was a successful debuff
			const float SourceDebuffChance = EffectSpec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Parameter_Chance, false, -1.f);

			// Get Target's resistance to our debuff(actually we get it from damage type since debuff's and damage's type is the same)
			float TargetDebuffResistance = 0.f;
			const FGameplayTag ResistanceTypeTag = FAuraGameplayTags::Get().DamageTypesToResistances[DamageTypeTag];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TagsToCaptureDefs[ResistanceTypeTag], EvaluateParams, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);

			const float EffectiveDebuffChance = SourceDebuffChance * (100 - TargetDebuffResistance) / 100.f;

			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bDebuff)
			{
				FGameplayEffectContextHandle EffectContextHandle = EffectSpec.GetContext();
				
				UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(EffectContextHandle, true);
				UAuraAbilitySystemLibrary::SetDamageTypeTag(EffectContextHandle, DamageTypeTag);

				const float DebuffDamage = EffectSpec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Parameter_Damage, false, -1.f);
				const float DebuffDuration = EffectSpec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Parameter_Duration, false, -1);
				const float DebuffFrequency = EffectSpec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Debuff_Parameter_Frequency, false, -1);

				UAuraAbilitySystemLibrary::SetDebuffDamage(EffectContextHandle, DebuffDamage);
				UAuraAbilitySystemLibrary::SetDebuffDuration(EffectContextHandle, DebuffDuration);
				UAuraAbilitySystemLibrary::SetDebuffFrequency(EffectContextHandle, DebuffFrequency);
			}
		}
	}
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}

	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}
		
	
	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	
	const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParams.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	FGameplayEffectContextHandle EffectContextHandle = EffectSpec.GetContext();

	// Debuff
	DetermineDebuff(ExecutionParams, EffectSpec, EvaluateParams);
	
	// Getting the damage from SetByCaller
	float Damage = 0.f;
	for (const auto& Pair : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		float DamageTypeValue = EffectSpec.GetSetByCallerMagnitude(DamageTypeTag, false);

		// If you use DamageStatics, it's for some reason called before AuraGameplayTag initialized all the GameplayTags.
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(AuraDamageStatics().TagsToCaptureDefs.FindChecked(ResistanceTag), EvaluateParams, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue = (DamageTypeValue - DamageTypeValue * (Resistance) / 100.f);
		
		Damage += DamageTypeValue;
	}

	/** Critical Hit */
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluateParams, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.0f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluateParams, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.0f);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluateParams, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.0f);
	
	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);
	
	const float CurrentCriticalRandom = FMath::RandRange(0.f, 100.f);
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient; 
	if (CurrentCriticalRandom < EffectiveCriticalHitChance)
	{
		// Critical Hit is a double-damage + CriticalHitDamage
		UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, true);
		Damage = Damage * 2 + SourceCriticalHitDamage;
	}

	// BlockChance rules if the block is halved
	float BlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluateParams, BlockChance);
	BlockChance = FMath::Max<float>(BlockChance, 0.0f);

	const float CurrentBlockRandom = FMath::RandRange(0.0f, 100.0f);
	
	if (CurrentBlockRandom < BlockChance)
	{
		// Successful block! Halve the damage
		UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, true);
		Damage /= 2.0f;
	}

	float TargetArmor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParams, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.0f);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluateParams, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.0f);
	
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());

	// Calculating Effective armor. It depends both on the Target's Armor value and the Source's Armor Penetration(Armor ignorance) value
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;

	// Damage dealt is then reduced by the Effective Armor(Armor left)
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval( TargetPlayerLevel);
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;

	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Override, Damage);
	
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
