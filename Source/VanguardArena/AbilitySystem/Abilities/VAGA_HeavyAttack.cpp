// VAGA_HeavyAttack.cpp

#include "AbilitySystem/Abilities/VAGA_HeavyAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
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
	// Cost + Cooldown
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Montage kontrolü
	if (!HeavyAttackMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("HeavyAttack: Montage atanmamış!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// State sıfırla
	bHasReleased = false;
	HitActors.Empty();
	CurrentDamageMultiplier = 1.0f;

	// Charge zamanlayıcı başlat
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	// Montage'ı Charge section'ından başlat
	// Charge section loop olarak ayarlanmış — karakter o pozda kalır
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			HeavyAttackMontage,
			1.0f,                // PlayRate
			ChargeSectionName    // StartSection = "Charge"
		);

	MontageTask->OnCompleted.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCancelled);
	MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_HeavyAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// Hasar event'ini şimdiden dinlemeye başla
	// Release section'ındaki AnimNotify tetiklediğinde yakalanacak
	UAbilityTask_WaitGameplayEvent* EventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			FVAGameplayTags::Get().Event_Montage_MeleeHit,
			nullptr,
			true  // OnlyTriggerOnce
		);

	EventTask->EventReceived.AddDynamic(this, &UVAGA_HeavyAttack::OnMeleeHitEvent);
	EventTask->ReadyForActivation();

	// Max charge timer — süre dolunca otomatik release
	GetWorld()->GetTimerManager().SetTimer(
		MaxChargeTimerHandle,
		this,
		&UVAGA_HeavyAttack::OnMaxChargeReached,
		MaxChargeTime,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: CHARGE BAŞLADI"));
}

void UVAGA_HeavyAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Timer temizle
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MaxChargeTimerHandle);
	}

	HitActors.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ─── INPUT RELEASE — Oyuncu tuşu bıraktı ───
void UVAGA_HeavyAttack::OnInputReleased()
{
	if (bHasReleased) return;
	PerformRelease();
}

// ─── MAX CHARGE TIMEOUT ───
void UVAGA_HeavyAttack::OnMaxChargeReached()
{
	UE_LOG(LogTemp, Log, TEXT("HeavyAttack: MAX CHARGE — otomatik release"));
	if (!bHasReleased)
	{
		PerformRelease();
	}
}

// ─── RELEASE: Charge hesapla + vuruş section'ına atla ───
void UVAGA_HeavyAttack::PerformRelease()
{
	bHasReleased = true;

	// Timer iptal
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MaxChargeTimerHandle);
	}

	// Charge süresi hesapla
	float ChargeTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;

	// Minimum kontrol — çok kısa basıldıysa iptal
	if (ChargeTime < MinChargeTime)
	{
		UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Çok kısa (%.2fs) — İPTAL"), ChargeTime);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Hasar çarpanı hesapla
	if (ChargeTime >= FullChargeThreshold)
	{
		CurrentDamageMultiplier = Level2Multiplier;
		UE_LOG(LogTemp, Log, TEXT("HeavyAttack: FULL CHARGE (%.2fs) → x%.1f"), ChargeTime, CurrentDamageMultiplier);
	}
	else
	{
		CurrentDamageMultiplier = Level1Multiplier;
		UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Level 1 (%.2fs) → x%.1f"), ChargeTime, CurrentDamageMultiplier);
	}

	// ─── RELEASE SECTION'INA ATLA ───
	// Montage_JumpToSection: Charge loop'unu kırar, Release animasyonuna geçer
	ACharacter* Character = GetAvatarCharacter();
	if (Character && HeavyAttackMontage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_JumpToSection(ReleaseSectionName, HeavyAttackMontage);
			UE_LOG(LogTemp, Log, TEXT("HeavyAttack: Release section'ına atlandı"));
		}
	}
}

// ─── MONTAGE BİTTİ (Release animasyonu tamamlandı) ───
void UVAGA_HeavyAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

// ─── MONTAGE İPTAL (stun, ölüm vs.) ───
void UVAGA_HeavyAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

// ─── ANİMNOTIFY HASAR EVENT'İ GELDİ ───
void UVAGA_HeavyAttack::OnMeleeHitEvent(FGameplayEventData Payload)
{
	PerformTrace();
}

// ─── HASAR TRACE ───
void UVAGA_HeavyAttack::PerformTrace()
{
	ACharacter* AvatarCharacter = GetAvatarCharacter();
	if (!AvatarCharacter) return;

	FVector Start = AvatarCharacter->GetActorLocation();
	FVector Forward = AvatarCharacter->GetActorForwardVector();
	FVector End = Start + (Forward * TraceDistance);

	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarCharacter);

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		Start,
		End,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResults,
		true,
		FLinearColor::Yellow,
		FLinearColor::Blue,
		3.0f
	);

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

				// Hit stop — heavy attack için daha uzun
				AVACharacterBase* OwnerChar = Cast<AVACharacterBase>(AvatarCharacter);
				if (OwnerChar)
				{
					float StopDuration = (CurrentDamageMultiplier >= Level2Multiplier) ? 0.12f : 0.08f;
					OwnerChar->ApplyHitStop(StopDuration, 0.01f);
				}

				UE_LOG(LogTemp, Log, TEXT("HeavyAttack HIT: %s (x%.1f)"),
					*HitActor->GetName(), CurrentDamageMultiplier);
			}
		}
	}
}

// ─── HEDEFE HASAR UYGULA ───
void UVAGA_HeavyAttack::ApplyDamageToTarget(AActor* Target)
{
	if (!DamageEffect) return;

	UAbilitySystemComponent* SourceASC = GetAvatarASC();
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (!SourceASC || !TargetASC) return;

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		DamageEffect, 1, EffectContext);

	if (SpecHandle.IsValid())
	{
		// Charge çarpanını level olarak uygula
		if (CurrentDamageMultiplier != 1.0f)
		{
			SpecHandle.Data->SetLevel(CurrentDamageMultiplier);
		}

		SourceASC->ApplyGameplayEffectSpecToTarget(
			*SpecHandle.Data.Get(), TargetASC);
	}
}