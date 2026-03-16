// VAGA_Dodge.cpp
#include "AbilitySystem/Abilities/VAGA_Dodge.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Combat/VATargetLockComponent.h"
#include "Combat/VACombatComponent.h"
#include "VAGameplayTags.h"

UVAGA_Dodge::UVAGA_Dodge()
{
	ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Dodging);
	CancelAbilitiesWithTag.AddTag(FVAGameplayTags::Get().State_Attacking);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);
	ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dodging);
}

void UVAGA_Dodge::ActivateAbility(
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

	ACharacter* Character = GetAvatarCharacter();
	if (!Character || !DodgeMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// State sıfırla
	DodgeElapsed = 0.0f;
	SavedDodgeDirection = CalculateDodgeDirection();

	// Lock-on DEĞİLSE karakteri dodge yönüne döndür
	UVATargetLockComponent* LockComp = Character->FindComponentByClass<UVATargetLockComponent>();
	bool bLocked = LockComp && LockComp->IsLockedOn();
	if (!bLocked && !SavedDodgeDirection.IsNearlyZero())
	{
		Character->SetActorRotation(FRotator(0.0f, SavedDodgeDirection.Rotation().Yaw, 0.0f));
	}

	// Mevcut hızı sıfırla — temiz başlangıç
	Character->GetCharacterMovement()->StopMovementImmediately();

	// Montage oynat — root motion hareketi DEVRE DIŞI
	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, DodgeMontage, PlayRate,
			NAME_None,  // StartSection
			true,       // bStopWhenAbilityEnds
			0.0f        // AnimRootMotionTranslationScale = 0
		);

	Task->OnCompleted.AddDynamic(this, &UVAGA_Dodge::OnMontageCompleted);
	Task->OnBlendOut.AddDynamic(this, &UVAGA_Dodge::OnMontageCompleted);
	Task->OnCancelled.AddDynamic(this, &UVAGA_Dodge::OnMontageCancelled);
	Task->OnInterrupted.AddDynamic(this, &UVAGA_Dodge::OnMontageCancelled);
	Task->ReadyForActivation();

	// Hareket timer başlat — her frame velocity ayarla
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &UVAGA_Dodge::DodgeMovementTick);
	GetWorld()->GetTimerManager().SetTimer(DodgeMovementTimer, TimerDel, 0.016f, true);

	UE_LOG(LogTemp, Log, TEXT("Dodge: Activated dir=(%s) speed=%.0f duration=%.2f"),
		*SavedDodgeDirection.ToCompactString(), DodgeSpeed, DodgeDuration);
}

void UVAGA_Dodge::DodgeMovementTick()
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character)
	{
		if (UWorld* World = GetWorld())
			World->GetTimerManager().ClearTimer(DodgeMovementTimer);
		return;
	}

	DodgeElapsed += 0.016f;

	if (DodgeElapsed >= DodgeDuration)
	{
		if (UWorld* World = GetWorld())
			World->GetTimerManager().ClearTimer(DodgeMovementTimer);
		return;
	}

	// Her frame velocity ayarla — bu input engelinden etkilenmez
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		FVector DodgeVelocity = SavedDodgeDirection * DodgeSpeed;
		DodgeVelocity.Z = MoveComp->Velocity.Z; // yerçekimi koru
		MoveComp->Velocity = DodgeVelocity;
	}
}

void UVAGA_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Timer temizle
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeMovementTimer);
	}

	// Dodge bitti — hızı sıfırla, kayma olmasın
	ACharacter* Character = GetAvatarCharacter();
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}

	// i-Frame tag temizliği
	UAbilitySystemComponent* ASC = GetAvatarASC();
	if (ASC && ASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Invincible))
	{
		ASC->RemoveLooseGameplayTag(FVAGameplayTags::Get().State_Invincible);
	}

	UE_LOG(LogTemp, Log, TEXT("Dodge: EndAbility"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UVAGA_Dodge::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("Dodge: Montage completed"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_Dodge::OnMontageCancelled()
{
	UE_LOG(LogTemp, Log, TEXT("Dodge: Montage cancelled"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

FVector UVAGA_Dodge::CalculateDodgeDirection() const
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return FVector::ForwardVector;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC) return Character->GetActorForwardVector();

	// Kameraya göre ileri ve sağ
	const FRotator CtrlRot(0, PC->GetControlRotation().Yaw, 0);
	const FVector CamForward = FRotationMatrix(CtrlRot).GetUnitAxis(EAxis::X);
	const FVector CamRight = FRotationMatrix(CtrlRot).GetUnitAxis(EAxis::Y);

	// CombatComponent'ten cached input oku (dodge input'undan ÖNCE kaydedildi)
	UVACombatComponent* CombatComp = Character->FindComponentByClass<UVACombatComponent>();
	if (CombatComp && CombatComp->bHasDodgeInput && !CombatComp->CachedDodgeInput.IsNearlyZero())
	{
		FVector2D Input = CombatComp->CachedDodgeInput;
		FVector Dir = (CamForward * Input.Y) + (CamRight * Input.X);
		Dir.Z = 0.0f;
		CombatComp->ClearDodgeInput();

		if (!Dir.IsNearlyZero())
		{
			UE_LOG(LogTemp, Log, TEXT("Dodge: Cached input (%.1f, %.1f)"), Input.X, Input.Y);
			return Dir.GetSafeNormal();
		}
	}

	// Son hareket input'u
	FVector LastInput = Character->GetCharacterMovement()->GetLastInputVector();
	if (!LastInput.IsNearlyZero())
	{
		LastInput.Z = 0.0f;
		return LastInput.GetSafeNormal();
	}

	// Kameranın ileri yönü
	return CamForward;
}