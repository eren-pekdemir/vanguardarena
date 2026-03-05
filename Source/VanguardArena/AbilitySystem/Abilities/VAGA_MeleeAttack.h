// VAGA_MeleeAttack.h
// Tüm melee saldırı ability'lerinin C++ base sınıfı

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_MeleeAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;

UCLASS(Abstract)
class VANGUARDARENA_API UVAGA_MeleeAttack : public UVAGameplayAbility
{
    GENERATED_BODY()

public:
    UVAGA_MeleeAttack();

protected:
    
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    TObjectPtr<UAnimMontage> AttackMontage;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    TSubclassOf<UGameplayEffect> DamageEffect;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    float TraceRadius = 50.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    float TraceDistance = 150.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    float MontagePlayRate = 1.0f;
    
    UFUNCTION()
    void OnMontageCompleted();
    
    UFUNCTION()
    void OnMontageCancelled();
    
    UFUNCTION()
    void OnMeleeHitEvent(FGameplayEventData Payload);
    
    void PerformMeleeTrace();
    
    void ApplyDamageToTarget(AActor* Target);

private:
    UPROPERTY()
    TSet<TObjectPtr<AActor>> HitActors;
};