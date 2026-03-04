// Fill out your copyright notice in the Description page of Project Settings.

#include "Input/VAInputConfig.h"

const FGameplayTag& UVAInputConfig::FindTagForAction(const UInputAction* Action) const
{
	for (const FVAInputAction& IAMapping : AbilityActions)
	{
		if (IAMapping.InputAction == Action && IAMapping.InputTag.IsValid())
		{
			return IAMapping.InputTag;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("VAInputConfig: Action için tag bulunamadı!"));
	return FGameplayTag::EmptyTag;
}