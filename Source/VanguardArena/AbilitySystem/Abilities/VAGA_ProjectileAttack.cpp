// VAGA_ProjectileAttack.cpp
#include "AbilitySystem/Abilities/VAGA_ProjectileAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "Combat/VAProjectile.h"
#include "VAGameplayTags.h"
#include "Combat/VATargetLockComponent.h"

UVAGA_ProjectileAttack::UVAGA_ProjectileAttack()
{
	ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Attacking);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);
}

void UVAGA_ProjectileAttack::ActivateAbility(
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

	// Montage oynat (fırlatma animasyonu)
	if (FireMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, FireMontage, PlayRate);

		MontageTask->OnCompleted.AddDynamic(this, &UVAGA_ProjectileAttack::OnMontageCompleted);
		MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_ProjectileAttack::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UVAGA_ProjectileAttack::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_ProjectileAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();

		// AnimNotify event dinle — mermi fırlatma anı
		UAbilityTask_WaitGameplayEvent* EventTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, FVAGameplayTags::Get().Event_Montage_SpawnProjectile,
				nullptr, true);
		EventTask->EventReceived.AddDynamic(this, &UVAGA_ProjectileAttack::OnSpawnProjectileEvent);
		EventTask->ReadyForActivation();
	}
	else
	{
		// Montage yok — doğrudan fırlat
		SpawnProjectile();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UVAGA_ProjectileAttack::OnSpawnProjectileEvent(FGameplayEventData Payload)
{
	SpawnProjectile();
}

void UVAGA_ProjectileAttack::SpawnProjectile()
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character || !ProjectileClass) return;

	// Spawn konumu — karakterin önünde
	FVector SpawnLocation = Character->GetActorLocation()
		+ Character->GetActorForwardVector() * SpawnOffset.X
		+ Character->GetActorRightVector() * SpawnOffset.Y
		+ FVector(0, 0, SpawnOffset.Z);

	FRotator SpawnRotation = Character->GetActorRotation();

	// Lock-on aktifse → hedefe doğru fırlat
	// (TargetLockComponent'ten hedef al)
	UVATargetLockComponent* LockComp = Character->FindComponentByClass<UVATargetLockComponent>();
	if (LockComp && LockComp->IsLockedOn() && LockComp->GetLockedTarget())
	{
		FVector TargetLoc = LockComp->GetLockedTarget()->GetActorLocation() + FVector(0, 0, 50);
		SpawnRotation = (TargetLoc - SpawnLocation).Rotation();
	}

	// Spawn
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AVAProjectile* Projectile = GetWorld()->SpawnActor<AVAProjectile>(
		ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Projectile)
	{
		// Hasar spec oluştur ve mermiye ver
		Projectile->OwnerActor = Character;

		UAbilitySystemComponent* SourceASC = GetAvatarASC();
		if (SourceASC && DamageEffect)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(this);
			Projectile->DamageSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffect, 1, Context);
		}

		UE_LOG(LogTemp, Log, TEXT("Projectile spawned → %s"), *Projectile->GetName());
	}
}

void UVAGA_ProjectileAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_ProjectileAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}