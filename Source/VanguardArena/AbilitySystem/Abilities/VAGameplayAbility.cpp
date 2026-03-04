#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "VAGameplayTags.h"

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