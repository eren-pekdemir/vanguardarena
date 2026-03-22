// VAGA_ChannelAbility.cpp
#include "AbilitySystem/Abilities/VAGA_ChannelAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "VAGameplayTags.h"
#include "Engine/OverlapResult.h"

UVAGA_ChannelAbility::UVAGA_ChannelAbility()
{
	// Kanal sırasında State.Channeling tag
	ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Channeling);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);

	// Hasar alınca kanal kesilsin
	// State.Channeling tag'i ActivationOwnedTags'ta
	// Hit React ability CancelAbilitiesWithTag'te State.Channeling eklenebilir
}

void UVAGA_ChannelAbility::ActivateAbility(
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

	bChannelEnding = false;

	// Montage (looping kanal animasyonu)
	if (ChannelMontage)
	{
		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, ChannelMontage, 1.0f);
		Task->OnCompleted.AddDynamic(this, &UVAGA_ChannelAbility::OnMontageCompleted);
		Task->OnBlendOut.AddDynamic(this, &UVAGA_ChannelAbility::OnMontageCompleted);
		Task->OnCancelled.AddDynamic(this, &UVAGA_ChannelAbility::OnMontageCancelled);
		Task->OnInterrupted.AddDynamic(this, &UVAGA_ChannelAbility::OnMontageCancelled);
		Task->ReadyForActivation();
	}

	// Periyodik tick timer
	FTimerDelegate TickDel;
	TickDel.BindUObject(this, &UVAGA_ChannelAbility::ChannelTick);
	GetWorld()->GetTimerManager().SetTimer(ChannelTickTimer, TickDel, TickInterval, true);

	// Max süre timer
	FTimerDelegate MaxDel;
	MaxDel.BindLambda([this]()
	{
		UE_LOG(LogTemp, Log, TEXT("Channel: Max süre doldu"));
		if (!bChannelEnding)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	});
	GetWorld()->GetTimerManager().SetTimer(MaxChannelTimer, MaxDel, MaxChannelTime, false);

	UE_LOG(LogTemp, Log, TEXT("Channel: BAŞLADI (max %.1fs, tick %.1fs)"), MaxChannelTime, TickInterval);
}

void UVAGA_ChannelAbility::ChannelTick()
{
	if (!ChannelEffect) return;

	UAbilitySystemComponent* SourceASC = GetAvatarASC();
	if (!SourceASC) return;

	if (bIsAOE)
	{
		// AOE — etraftaki düşmanlara uygula
		TArray<AActor*> Ignore;
		Ignore.Add(GetAvatarActorFromActorInfo());
		ApplyAOEDamage(GetAvatarCharacter()->GetActorLocation(), AOERadius, ChannelEffect, Ignore);
	}
	else
	{
		// Self — kendine uygula (heal, regen vs.)
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(ChannelEffect, 1, Context);
		if (Spec.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void UVAGA_ChannelAbility::OnInputReleased()
{
	UE_LOG(LogTemp, Log, TEXT("Channel: Input RELEASED"));
	if (!bChannelEnding)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UVAGA_ChannelAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bChannelEnding) return;
	bChannelEnding = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChannelTickTimer);
		World->GetTimerManager().ClearTimer(MaxChannelTimer);
	}

	UE_LOG(LogTemp, Log, TEXT("Channel: BİTTİ (cancelled=%s)"), bWasCancelled ? TEXT("true") : TEXT("false"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UVAGA_ChannelAbility::OnMontageCompleted()
{
	if (!bChannelEnding)
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_ChannelAbility::OnMontageCancelled()
{
	if (!bChannelEnding)
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}