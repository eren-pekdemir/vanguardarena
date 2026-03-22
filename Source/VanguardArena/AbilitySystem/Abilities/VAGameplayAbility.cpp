#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "VAGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"

UVAGameplayAbility::UVAGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UVAGameplayAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UE_LOG(LogTemp, Log, TEXT("VA Ability Activated: %s"), *GetName());
}

void UVAGameplayAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    if (bWasCancelled)
    {
        UE_LOG(LogTemp, Log, TEXT("VA Ability CANCELLED: %s"), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("VA Ability Ended: %s"), *GetName());
    }
}

ACharacter* UVAGameplayAbility::GetAvatarCharacter() const
{
    return Cast<ACharacter>(GetAvatarActorFromActorInfo());
}

UAbilitySystemComponent* UVAGameplayAbility::GetAvatarASC() const
{
    return GetAbilitySystemComponentFromActorInfo();
}

void UVAGameplayAbility::ApplyAOEDamage(
    const FVector& Center,
    float Radius,
    TSubclassOf<UGameplayEffect> EffectClass,
    const TArray<AActor*>& IgnoreActors)
{
    if (!EffectClass) return;

    UAbilitySystemComponent* SourceASC = GetAvatarASC();
    if (!SourceASC) return;

    // Alan taraması
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);

    GetWorld()->OverlapMultiByChannel(
        Overlaps, Center, FQuat::Identity,
        ECollisionChannel::ECC_Pawn, Shape);

    // Effect spec oluştur
    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
    Context.AddSourceObject(this);
    FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(EffectClass, 1, Context);

    if (!Spec.IsValid()) return;

    int32 HitCount = 0;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor) continue;
        if (HitActor == GetAvatarActorFromActorInfo()) continue;
        if (IgnoreActors.Contains(HitActor)) continue;

        UAbilitySystemComponent* TargetASC =
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);

        if (TargetASC)
        {
            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
            HitCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AOE Damage: Radius=%.0f Hits=%d"), Radius, HitCount);
}