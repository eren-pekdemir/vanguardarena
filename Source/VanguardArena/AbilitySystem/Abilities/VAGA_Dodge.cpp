// VAGA_Dodge.cpp
#include "AbilitySystem/Abilities/VAGA_Dodge.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Combat/VATargetLockComponent.h"
#include "VAGameplayTags.h"

UVAGA_Dodge::UVAGA_Dodge()
{
    // ─── TAG AYARLARI ───

    // Dodge sırasında bu tag'ler karakterde olur
    // State.Invincible → AnimNotifyState ile kontrol ediliyor (BÖLÜM C)
    // Burada sadece dodge sırasında saldırıyı iptal ediyoruz

    // Ölü, stun, zaten dodge yapıyorken tekrar dodge yapamaz
    ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
    ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);

    // Dodge saldırıyı iptal eder (saldırı sırasında dodge yapılabilir)
    CancelAbilitiesWithTag.AddTag(FVAGameplayTags::Get().State_Attacking);
}

void UVAGA_Dodge::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    // Cost + Cooldown kontrol
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

    // ─── DODGE YÖNÜ ───
    FVector DodgeDirection = CalculateDodgeDirection();

    // Karakteri dodge yönüne döndür (animasyon ileri yönde oynar)
    if (!DodgeDirection.IsNearlyZero())
    {
        FRotator DodgeRotation = DodgeDirection.Rotation();
        Character->SetActorRotation(FRotator(0.0f, DodgeRotation.Yaw, 0.0f));
    }

    // ─── ROOT MOTION YOKSA: LaunchCharacter ───
    if (!bUseRootMotion)
    {
        FVector LaunchVelocity = DodgeDirection * (DodgeDistance / DodgeDuration);
        LaunchVelocity.Z = 0.0f; // Yatay düzlemde kal
        Character->LaunchCharacter(LaunchVelocity, true, true);
    }

    // ─── MONTAGE OYNAT ───
    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, DodgeMontage, PlayRate);

    MontageTask->OnCompleted.AddDynamic(this, &UVAGA_Dodge::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UVAGA_Dodge::OnMontageCancelled);
    MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_Dodge::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_Dodge::OnMontageCancelled);
    MontageTask->ReadyForActivation();

    UE_LOG(LogTemp, Log, TEXT("Dodge: Yön=(%s) Mesafe=%.0f"),
        *DodgeDirection.ToString(), DodgeDistance);
}

void UVAGA_Dodge::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // i-Frame tag'inin kaldırıldığından emin ol (safety net)
    // Normalde AnimNotifyState kaldırır ama ability iptal edilirse
    // notify çalışmayabilir
    UAbilitySystemComponent* ASC = GetAvatarASC();
    if (ASC && ASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Invincible))
    {
        ASC->RemoveLooseGameplayTag(FVAGameplayTags::Get().State_Invincible);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UVAGA_Dodge::CalculateDodgeDirection() const
{
    ACharacter* Character = GetAvatarCharacter();
    if (!Character) return FVector::ForwardVector;

    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (!PC) return Character->GetActorForwardVector();

    // ─── LOCK-ON AKTİF Mİ? ───
    UVATargetLockComponent* LockComp = Character->FindComponentByClass<UVATargetLockComponent>();
    bool bLockedOn = LockComp && LockComp->IsLockedOn();

    if (bLockedOn)
    {
        // Lock-on modunda: WASD input yönüne göre dodge
        // W → hedefe doğru, S → hedeften uzağa, A → sola, D → sağa

        // Son hareket input'unu al
        // GetLastMovementInputVector: son frame'deki hareket yönü
        FVector InputDirection = Character->GetCharacterMovement()->GetLastInputVector();

        if (!InputDirection.IsNearlyZero())
        {
            return InputDirection.GetSafeNormal();
        }

        // Input yoksa → hedeften uzağa dodge (geri)
        if (LockComp->GetLockedTarget())
        {
            FVector AwayFromTarget = (Character->GetActorLocation() -
                LockComp->GetLockedTarget()->GetActorLocation()).GetSafeNormal();
            AwayFromTarget.Z = 0.0f;
            return AwayFromTarget;
        }
    }

    // Lock-on kapalı veya hedef yok: hareket input yönüne veya ileri yönüne dodge
    FVector InputDirection = Character->GetCharacterMovement()->GetLastInputVector();
    if (!InputDirection.IsNearlyZero())
    {
        return InputDirection.GetSafeNormal();
    }

    // Hiç input yoksa → ileri yönde dodge
    return Character->GetActorForwardVector();
}

void UVAGA_Dodge::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_Dodge::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}