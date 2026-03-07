// VAGA_MeleeAttack.cpp
#include "AbilitySystem/Abilities/VAGA_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Combat/VACombatComponent.h"
#include "Combat/VAComboData.h"
#include "VAGameplayTags.h"

UVAGA_MeleeAttack::UVAGA_MeleeAttack()
{
    ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Attacking);
    ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
    ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);
}

UVACombatComponent* UVAGA_MeleeAttack::GetCombatComponent() const
{
    if (CachedCombatComp.IsValid())
    {
        return CachedCombatComp.Get();
    }

    ACharacter* Character = GetAvatarCharacter();
    if (Character)
    {
        return Character->FindComponentByClass<UVACombatComponent>();
    }
    return nullptr;
}

void UVAGA_MeleeAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UVACombatComponent* CombatComp = GetCombatComponent();
    
    bool bIsFirstHit = !CombatComp || CombatComp->ComboIndex == 0;
    
    if (bIsFirstHit)
    {
        if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
        {
            if (CombatComp) CombatComp->ResetCombo();
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
    }

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    CachedCombatComp = CombatComp;
    
    CurrentMontage = AttackMontage;
    CurrentDamageMultiplier = 1.0f;
    CurrentPlayRate = 1.0f;
    CurrentTraceRadius = TraceRadius;
    CurrentTraceDistance = TraceDistance;
    CurrentDamageEffect = DamageEffect;
    
    if (ComboData && CombatComp)
    {
        int32 Index = CombatComp->ComboIndex;
        const FVAComboAttack* ComboAttack = ComboData->GetComboAttack(Index);

        if (ComboAttack)
        {
            if (ComboAttack->AttackMontage)
            {
                CurrentMontage = ComboAttack->AttackMontage;
            }
            
            CurrentDamageMultiplier = ComboAttack->DamageMultiplier;
            
            CurrentPlayRate = ComboAttack->PlayRate;
            
            if (ComboAttack->OverrideDamageEffect)
            {
                CurrentDamageEffect = ComboAttack->OverrideDamageEffect;
            }
            
            if (ComboAttack->OverrideTraceRadius > 0.0f)
            {
                CurrentTraceRadius = ComboAttack->OverrideTraceRadius;
            }
            if (ComboAttack->OverrideTraceDistance > 0.0f)
            {
                CurrentTraceDistance = ComboAttack->OverrideTraceDistance;
            }

            UE_LOG(LogTemp, Log, TEXT("Combo[%d]: Montage=%s DmgMult=%.1f Rate=%.1f"),
                Index,
                *CurrentMontage->GetName(),
                CurrentDamageMultiplier,
                CurrentPlayRate);
        }
    }
    
    if (!CurrentMontage)
    {
        UE_LOG(LogTemp, Error, TEXT("GA_MeleeAttack: Montage YOK!"));
        if (CombatComp) CombatComp->ResetCombo();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    HitActors.Empty();
    
    if (CombatComp)
    {
        CombatComp->bIsAttacking = true;
        CombatComp->bWantsCombo = false;
        CombatComp->bComboWindowOpen = false;
    }
    
    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            CurrentMontage,
            CurrentPlayRate
        );

    MontageTask->OnCompleted.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCancelled);
    MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCancelled);
    MontageTask->ReadyForActivation();
    
    UAbilityTask_WaitGameplayEvent* EventTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            FVAGameplayTags::Get().Event_Montage_MeleeHit,
            nullptr,
            false  
        );

    EventTask->EventReceived.AddDynamic(this, &UVAGA_MeleeAttack::OnMeleeHitEvent);
    EventTask->ReadyForActivation();
}

void UVAGA_MeleeAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    HitActors.Empty();

    UVACombatComponent* CombatComp = GetCombatComponent();

    if (CombatComp)
    {
        CombatComp->bIsAttacking = false;

        if (bWasCancelled)
        {
            CombatComp->ResetCombo();
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UVAGA_MeleeAttack::OnMontageCompleted()
{
    UVACombatComponent* CombatComp = GetCombatComponent();

    if (CombatComp && CombatComp->bWantsCombo)
    {
        CombatComp->AdvanceCombo();
    }
    else
    {
        // Combo devam yok → sıfırla
        if (CombatComp) CombatComp->ResetCombo();
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_MeleeAttack::OnMontageCancelled()
{
    // İptal → combo sıfırla (EndAbility'de yapılır)
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UVAGA_MeleeAttack::OnMeleeHitEvent(FGameplayEventData Payload)
{
    PerformMeleeTrace();
}

void UVAGA_MeleeAttack::PerformMeleeTrace()
{
    ACharacter* AvatarCharacter = GetAvatarCharacter();
    if (!AvatarCharacter) return;

    FVector Start = AvatarCharacter->GetActorLocation();
    FVector Forward = AvatarCharacter->GetActorForwardVector();
    FVector End = Start + (Forward * CurrentTraceDistance);

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarCharacter);

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(),
        Start,
        End,
        CurrentTraceRadius,
        UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::ForDuration,
        HitResults,
        true,
        FLinearColor::Red,
        FLinearColor::Green,
        2.0f
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
                UE_LOG(LogTemp, Log, TEXT("Combo[%d] HIT: %s (x%.1f dmg)"),
                    CachedCombatComp.IsValid() ? CachedCombatComp->ComboIndex : 0,
                    *HitActor->GetName(),
                    CurrentDamageMultiplier);
            }
        }
    }
}

void UVAGA_MeleeAttack::ApplyDamageToTarget(AActor* Target)
{
    if (!CurrentDamageEffect) return;

    UAbilitySystemComponent* SourceASC = GetAvatarASC();
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

    if (!SourceASC || !TargetASC) return;

    FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
        CurrentDamageEffect,
        1, // Level
        EffectContext
    );

    if (SpecHandle.IsValid())
    {
        if (CurrentDamageMultiplier != 1.0f)
        {
            SpecHandle.Data->SetLevel(CurrentDamageMultiplier);
        }

        SourceASC->ApplyGameplayEffectSpecToTarget(
            *SpecHandle.Data.Get(),
            TargetASC
        );
    }
}