// VAGA_SmashAttack.cpp
#include "AbilitySystem/Abilities/VAGA_SmashAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VAGameplayTags.h"
#include "Engine/OverlapResult.h"

UVAGA_SmashAttack::UVAGA_SmashAttack()
{
	ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Attacking);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);
}

void UVAGA_SmashAttack::ActivateAbility(
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

	if (!SmashMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Montage oynat
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, SmashMontage, PlayRate);

	MontageTask->OnCompleted.AddDynamic(this, &UVAGA_SmashAttack::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_SmashAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UVAGA_SmashAttack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_SmashAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// Smash anı event'ini dinle
	UAbilityTask_WaitGameplayEvent* EventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, FVAGameplayTags::Get().Event_Montage_Smash, nullptr, true);
	EventTask->EventReceived.AddDynamic(this, &UVAGA_SmashAttack::OnSmashEvent);
	EventTask->ReadyForActivation();
}

void UVAGA_SmashAttack::OnSmashEvent(FGameplayEventData Payload)
{
	PerformSmash();
}

void UVAGA_SmashAttack::PerformSmash()
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;

	FVector SmashCenter = Character->GetActorLocation();
	UAbilitySystemComponent* SourceASC = GetAvatarASC();

	// ─── AOE HASAR ───
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Shape = FCollisionShape::MakeSphere(SmashRadius);

	GetWorld()->OverlapMultiByChannel(
		Overlaps, SmashCenter, FQuat::Identity,
		ECollisionChannel::ECC_Pawn, Shape);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == Character) continue;

		// Hasar uygula
		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);

		if (TargetASC && SourceASC && DamageEffect)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, 1, Context);
			if (Spec.IsValid())
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}

		// ─── LAUNCH (fırlatma) ───
		ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
		if (HitCharacter)
		{
			// Smash merkezinden uzağa doğru fırlat
			FVector LaunchDir = (HitActor->GetActorLocation() - SmashCenter).GetSafeNormal();
			LaunchDir.Z = 0.0f;
			FVector LaunchVelocity = LaunchDir * LaunchStrength + FVector(0, 0, LaunchUpStrength);
			HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
			
			PlayHitEffects(HitActor->GetActorLocation(), FVector::UpVector);
			
		}
	}
	
	
	// ─── CAMERA SHAKE ───
	if (SmashCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			SmashCameraShake,
			SmashCenter,
			500.0f,     // Inner radius (tam shake)
			1500.0f,    // Outer radius (shake azalır)
			1.0f        // Falloff
		);
	}

}

void UVAGA_SmashAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_SmashAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}