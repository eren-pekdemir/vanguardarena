// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "VAAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VANGUARDARENA_API UVAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UVAAbilitySystemComponent();
	
	void GiveStartupAbilities(const TArray<TSubclassOf<UGameplayAbility>> &Abilities);
	void ApplyStartupEffects(const TArray<TSubclassOf<UGameplayEffect>>& Effects);
	bool TryActivateAbilityByTag(FGameplayTag AbilityTag);
	void CancelAllActiveAbilities();	
	bool IsAbilityActiveByTag(FGameplayTag AbilityTag) const;
protected:
	TArray<FGameplayAbilitySpecHandle> GivenAbilityHandles;
	TArray<FActiveGameplayEffectHandle> AppliedEffectHandles;
	bool bStartupAbilitiesGiven = false;
};


