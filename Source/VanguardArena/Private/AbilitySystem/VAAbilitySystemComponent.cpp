// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/VAAbilitySystemComponent.h"

UVAAbilitySystemComponent::UVAAbilitySystemComponent()
{
	SetIsReplicatedByDefault( true);
	ReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

void UVAAbilitySystemComponent::GiveStartupAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	if (bStartupAbilitiesGiven) return;
	
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		if (!AbilityClass) continue;
		
		FGameplayAbilitySpec AbilitySpec(AbilityClass,1, INDEX_NONE, GetOwner());
		FGameplayAbilitySpecHandle AbilityHandle = GiveAbility(AbilitySpec);
		GivenAbilityHandles.Add(AbilityHandle);
	}
	bStartupAbilitiesGiven = true;
}

void UVAAbilitySystemComponent::ApplyStartupEffects(const TArray<TSubclassOf<UGameplayEffect>>& Effects)
{
	for (const TSubclassOf<UGameplayEffect>& EffectClass : Effects)
	{
		if (!EffectClass) continue;
		
		FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
		ContextHandle.AddSourceObject(GetOwner());
		
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, ContextHandle);
		
		if (!EffectSpecHandle.IsValid()) continue;
		
		FActiveGameplayEffectHandle EffectHandle = ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		AppliedEffectHandles.Add(EffectHandle);
	}
}

bool UVAAbilitySystemComponent::TryActivateAbilityByTag(FGameplayTag AbilityTag)
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(AbilityTag);
	
	return TryActivateAbilitiesByTag(TagContainer);
}

void UVAAbilitySystemComponent::CancelAllActiveAbilities()
{
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.IsActive())
		{
				CancelAbilityHandle(Spec.Handle);
		}
	}
}

bool UVAAbilitySystemComponent::IsAbilityActiveByTag(FGameplayTag AbilityTag) const
{
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.Ability->GetAssetTags().HasTag(AbilityTag))
		{
			return true;
		}
	}
	return false;
}
