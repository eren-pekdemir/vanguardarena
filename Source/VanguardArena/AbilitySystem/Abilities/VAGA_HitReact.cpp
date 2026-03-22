// VAGA_HitReact.cpp
#include "AbilitySystem/Abilities/VAGA_HitReact.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "VAGameplayTags.h"

UVAGA_HitReact::UVAGA_HitReact()
{
    // Hit react otomatik aktive olur (Event ile tetiklenir)
    // Input gerekmez — hasar gelince otomatik çalışır
    ActivationPolicy = EVAAbilityActivationPolicy::OnGiven;

    // Ölüyken hit react yok
    ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);

    // Hit react sırasında SALDIRI iptal olsun
    // Böylece düşman vurulunca saldırısı kesilir
    CancelAbilitiesWithTag.AddTag(FVAGameplayTags::Get().State_Attacking);
    CancelAbilitiesWithTag.AddTag(FVAGameplayTags::Get().State_Channeling);
}

void UVAGA_HitReact::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Character = GetAvatarCharacter();
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ─── VURUŞ YÖNÜNÜ BELİRLE ───
    // TriggerEventData içinde Instigator (saldıran) bilgisi var
    // Saldıranın konumundan vuruş yönü hesaplanır
    FVector HitDirection = FVector::ForwardVector; // default: önden

    if (TriggerEventData && TriggerEventData->Instigator != nullptr)
    {
        FVector InstigatorLocation = TriggerEventData->Instigator->GetActorLocation();
        FVector VictimLocation = Character->GetActorLocation();
        HitDirection = (InstigatorLocation - VictimLocation).GetSafeNormal();
    }

    // Doğru montage'ı seç
    UAnimMontage* SelectedMontage = SelectMontageByDirection(HitDirection);

    if (!SelectedMontage)
    {
        // Hiçbir montage atanmamış — ability'yi bitir
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Montage oynat
    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, SelectedMontage, 1.0f);

    MontageTask->OnCompleted.AddDynamic(this, &UVAGA_HitReact::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UVAGA_HitReact::OnMontageCancelled);
    MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_HitReact::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_HitReact::OnMontageCancelled);
    MontageTask->ReadyForActivation();

    UE_LOG(LogTemp, Log, TEXT("HitReact: %s montage oynatılıyor"), *SelectedMontage->GetName());
}

UAnimMontage* UVAGA_HitReact::SelectMontageByDirection(const FVector& HitDirection) const
{
    ACharacter* Character = GetAvatarCharacter();
    if (!Character) return HitReactFront;

    // Karakterin ileri ve sağ vektörleri
    FVector CharForward = Character->GetActorForwardVector();
    FVector CharRight = Character->GetActorRightVector();

    // Dot product: vuruş yönü ile karakter yönlerinin açısı
    float DotForward = FVector::DotProduct(CharForward, HitDirection);
    float DotRight = FVector::DotProduct(CharRight, HitDirection);

    // Hangi yön baskın?
    // DotForward > 0 → vuruş ÖNDEN geliyor
    // DotForward < 0 → vuruş ARKADAN geliyor
    // DotRight > 0 → vuruş SAĞDAN geliyor
    // DotRight < 0 → vuruş SOLDAN geliyor

    if (FMath::Abs(DotForward) > FMath::Abs(DotRight))
    {
        // İleri/Geri baskın
        if (DotForward > 0.0f)
        {
            return HitReactFront ? HitReactFront : HitReactBack;
        }
        else
        {
            return HitReactBack ? HitReactBack : HitReactFront;
        }
    }
    else
    {
        // Sağ/Sol baskın
        if (DotRight > 0.0f)
        {
            return HitReactRight ? HitReactRight : HitReactLeft;
        }
        else
        {
            return HitReactLeft ? HitReactRight : HitReactRight;
        }
    }
}

void UVAGA_HitReact::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_HitReact::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}