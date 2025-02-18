// Copyright Abdu Inc.


#include "AbilitySystem/ModMagCalc/MMC_MaxMana.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"


UMMC_MaxMana::UMMC_MaxMana()
{
	IntelligenceCapture.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceCapture.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceCapture);
}


float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Intelligence = 0.0f;
	GetCapturedAttributeMagnitude(IntelligenceCapture, Spec, EvaluateParameters, Intelligence);

	Intelligence = FMath::Max<float>(0.0f, Intelligence);

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();

	return 50.0f + 2.5f * Intelligence + 15.0f * PlayerLevel;
}
