// VAGA_HeavyAttack.cpp
#include "AbilitySystem/Abilities/VAGA_HeavyAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "VAGameplayTags.h"
#include "Characters/VACharacterBase.h"

UVAGA_HeavyAttack::UVAGA_HeavyAttack()
{
	ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Attacking);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);
}

void UVAGA_HeavyAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// ─── TÜM STATE'İ SIFIRLA ───
	// InstancedPerActor: aynı obje tekrar kullanılır
	// Önceki kullanımdan kalan değerler temizlenmeli
	bHasReleased = false;
	HitActors.Empty();
	CurrentDamageMultiplier = 1.0f;
	ChargeStartTime = 0.0f;

	// Önceki timer varsa temizle
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MaxChargeTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("=== HeavyAttack: ActivateAbility BAŞLADI ==="));

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Error, TEXT("HeavyAttack: CommitAbility FAILED"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!HeavyAttackMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("HeavyAttack: Montage yok!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ChargeStartTime = GetWorld()->GetTimeSeconds();

	// Montage — WindUp section'ından başla
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, HeavyAttackMontage, 1.0f, WindUpSectionName);

	MontageTask->OnCompleted.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// Hasar event dinle
	UAbilityTask_WaitGameplayEvent* EventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, FVAGameplayTags::Get().Event_Montage_MeleeHit, nullptr, true);
	EventTask->EventReceived.AddDynamic(this, &UVAGA_HeavyAttack::OnMeleeHitEvent);
	EventTask->ReadyForActivation();

	// Max charge timer
	FTimerDelegate TimerDel;
	TimerDel.BindLambda([this]()
	{
		UE_LOG(LogTemp, Log, TEXT("HeavyAttack: MAX CHARGE timeout"));
		if (!bHasReleased)
		{
			PerformRelease();
		}
	});
	GetWorld()->GetTimerManager().SetTimer(MaxChargeTimerHandle, TimerDel, MaxChargeTime, false);

	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Charge başladı"));
}

void UVAGA_HeavyAttack::Cleanup()
{
	// Timer temizle
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MaxChargeTimerHandle);
	}

	// Hareketi geri aç (eğer kapalıysa)
	ACharacter* Character = GetAvatarCharacter();
	if (Character && Character->GetCharacterMovement())
	{
		if (Character->GetCharacterMovement()->MovementMode == MOVE_None)
		{
			Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Movement → Walking"));
		}
	}

	HitActors.Empty();
}

void UVAGA_HeavyAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Cleanup();

	UE_LOG(LogTemp, Warning, TEXT("=== HeavyAttack: EndAbility === (cancelled=%s)"),
		bWasCancelled ? TEXT("true") : TEXT("false"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UVAGA_HeavyAttack::OnInputReleased()
{
	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Input RELEASED (bHasReleased=%s)"),
		bHasReleased ? TEXT("true") : TEXT("false"));

	if (bHasReleased) return;
	PerformRelease();
}

void UVAGA_HeavyAttack::PerformRelease()
{
	bHasReleased = true;

	// Timer iptal
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MaxChargeTimerHandle);
	}

	float ChargeTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Release ChargeTime=%.2f"), ChargeTime);

	// Çok kısa — iptal
	if (ChargeTime < MinChargeTime)
	{
		UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Çok kısa — İPTAL"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Hasar çarpanı
	CurrentDamageMultiplier = (ChargeTime >= FullChargeThreshold) ? Level2Multiplier : Level1Multiplier;
	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Multiplier=%.1f"), CurrentDamageMultiplier);

	// Release section'ına atla
	ACharacter* Character = GetAvatarCharacter();
	if (Character && HeavyAttackMontage)
	{
		UAnimInstance* Anim = Character->GetMesh()->GetAnimInstance();
		if (Anim && Anim->Montage_IsPlaying(HeavyAttackMontage))
		{
			Anim->Montage_JumpToSection(ReleaseSectionName, HeavyAttackMontage);
			UE_LOG(LogTemp, Log, TEXT("HeavyAttack: JumpToSection('%s')"), *ReleaseSectionName.ToString());
			return; // Montage devam edecek, OnMontageCompleted'da EndAbility çağrılacak
		}
	}

	// Buraya geldiyse montage oynamıyor — doğrudan bitir
	UE_LOG(LogTemp, Error, TEXT("HeavyAttack: Montage oynamıyor, bitiriliyor"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UVAGA_HeavyAttack::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Montage COMPLETED"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_HeavyAttack::OnMontageCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Montage CANCELLED"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UVAGA_HeavyAttack::OnMeleeHitEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: MeleeHit event"));
	PerformTrace();
}

void UVAGA_HeavyAttack::PerformTrace()
{
	ACharacter* AvatarCharacter = GetAvatarCharacter();
	if (!AvatarCharacter) return;

	FVector Start = AvatarCharacter->GetActorLocation();
	FVector End = Start + (AvatarCharacter->GetActorForwardVector() * TraceDistance);

	TArray<FHitResult> HitResults;
	TArray<AActor*> Ignore;
	Ignore.Add(AvatarCharacter);

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(), Start, End, TraceRadius,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn),
		false, Ignore, EDrawDebugTrace::ForDuration,
		HitResults, true, FLinearColor::Yellow, FLinearColor::Blue, 3.0f);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActors.Contains(HitActor)) continue;

			UAbilitySystemComponent* TargetASC =
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
			if (TargetASC)
			{
				HitActors.Add(HitActor);
				ApplyDamageToTarget(HitActor);

				AVACharacterBase* Owner = Cast<AVACharacterBase>(AvatarCharacter);
				if (Owner)
				{
					Owner->ApplyHitStop(
						(CurrentDamageMultiplier >= Level2Multiplier) ? 0.12f : 0.08f, 0.01f);
				}
			}
		}
	}
}

void UVAGA_HeavyAttack::ApplyDamageToTarget(AActor* Target)
{
	if (!DamageEffect) return;

	UAbilitySystemComponent* SourceASC = GetAvatarASC();
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!SourceASC || !TargetASC) return;

	FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
	Ctx.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, 1, Ctx);
	if (Spec.IsValid())
	{
		if (CurrentDamageMultiplier != 1.0f)
			Spec.Data->SetLevel(CurrentDamageMultiplier);

		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}