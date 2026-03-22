// VAGA_AOEAttack.cpp
#include "AbilitySystem/Abilities/VAGA_AOEAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "VAGameplayTags.h"
#include "Engine/OverlapResult.h"

UVAGA_AOEAttack::UVAGA_AOEAttack()
{
	ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Attacking);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);
}

void UVAGA_AOEAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AOEMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, AOEMontage, PlayRate);

		Task->OnCompleted.AddDynamic(this, &UVAGA_AOEAttack::OnMontageCompleted);
		Task->OnBlendOut.AddDynamic(this, &UVAGA_AOEAttack::OnMontageCompleted);
		Task->OnCancelled.AddDynamic(this, &UVAGA_AOEAttack::OnMontageCancelled);
		Task->OnInterrupted.AddDynamic(this, &UVAGA_AOEAttack::OnMontageCancelled);
		Task->ReadyForActivation();

		// AOE tetikleme event'i dinle
		UAbilityTask_WaitGameplayEvent* EventTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, FVAGameplayTags::Get().Event_Montage_AOE, nullptr, true);
		EventTask->EventReceived.AddDynamic(this, &UVAGA_AOEAttack::OnAOEEvent);
		EventTask->ReadyForActivation();
	}
	else
	{
		PerformAOE();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UVAGA_AOEAttack::OnAOEEvent(FGameplayEventData Payload)
{
	PerformAOE();
}

void UVAGA_AOEAttack::PerformAOE()
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;

	FVector Center = Character->GetActorLocation() + AOECenterOffset;

	// Ana hasar
	TArray<AActor*> Ignore;
	Ignore.Add(Character);

	if (AOEDamageEffect)
	{
		ApplyAOEDamage(Center, AOERadius, AOEDamageEffect, Ignore);
	}

	// Ek effect'ler (DOT, stun vs.)
	for (const TSubclassOf<UGameplayEffect>& Effect : AdditionalAOEEffects)
	{
		if (Effect)
		{
			ApplyAOEDamage(Center, AOERadius, Effect, Ignore);
		}
	}

	// GameplayCue tetikle (patlama efekti)
	UAbilitySystemComponent* ASC = GetAvatarASC();
	if (ASC)
	{
		FGameplayCueParameters Params;
		Params.Location = Center;
		ASC->ExecuteGameplayCue(
			FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Ability.AOE")), Params);
	}
}

void UVAGA_AOEAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_AOEAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}