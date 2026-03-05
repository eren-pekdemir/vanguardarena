// VAGA_MeleeAttack.cpp
#include "AbilitySystem/Abilities/VAGA_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "VAGameplayTags.h"

UVAGA_MeleeAttack::UVAGA_MeleeAttack()
{
    ActivationOwnedTags.AddTag(FVAGameplayTags::Get().State_Attacking);

    ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Dead);
    ActivationBlockedTags.AddTag(FVAGameplayTags::Get().State_Stunned);
}

void UVAGA_MeleeAttack::ActivateAbility(
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
    
    HitActors.Empty();
    
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,                    
        NAME_None,            
        AttackMontage,         
        MontagePlayRate,        
        NAME_None,              
        false,                  
        1.0f                    
    );
    
    MontageTask->OnCompleted.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCancelled);
    MontageTask->OnBlendOut.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UVAGA_MeleeAttack::OnMontageCancelled);
    
    MontageTask->ReadyForActivation();

    UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this,                                         
        FVAGameplayTags::Get().Event_Montage_MeleeHit,  
        nullptr,                                       
        true                                            
    );
    
    EventTask->EventReceived.AddDynamic(this, &UVAGA_MeleeAttack::OnMeleeHitEvent);
    EventTask->ReadyForActivation();

    UE_LOG(LogTemp, Log, TEXT("GA_MeleeAttack: Montage başlatıldı + Event dinleniyor"));
}

void UVAGA_MeleeAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    HitActors.Empty();
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UVAGA_MeleeAttack::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UVAGA_MeleeAttack::OnMontageCancelled()
{
   
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
        FLinearColor::Red,   
        FLinearColor::Green, 
        2.0f            
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            
            if (!HitActor) continue;
            if (HitActors.Contains(HitActor)) continue; 
            
            UAbilitySystemComponent* TargetASC =
                UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);

            if (TargetASC)
            {
                HitActors.Add(HitActor);
                
                ApplyDamageToTarget(HitActor);

                UE_LOG(LogTemp, Log, TEXT("MeleeTrace HIT: %s"), *HitActor->GetName());
            }
        }
    }
}


void UVAGA_MeleeAttack::ApplyDamageToTarget(AActor* Target)
{
 
    if (!DamageEffect) return;

    UAbilitySystemComponent* SourceASC = GetAvatarASC();
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

    if (!SourceASC || !TargetASC) return;
    
    FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
    EffectContext.AddSourceObject(this); 
    
    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
        DamageEffect,  
        1,            
        EffectContext
    );

    if (SpecHandle.IsValid())
    {
        SourceASC->ApplyGameplayEffectSpecToTarget(
            *SpecHandle.Data.Get(),
            TargetASC
        );
    }
}