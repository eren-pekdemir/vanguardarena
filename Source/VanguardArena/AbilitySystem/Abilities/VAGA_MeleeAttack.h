#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_MeleeAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UVAComboDataAsset;
class UVACombatComponent;

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
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Combo")
    TObjectPtr<UVAComboDataAsset> ComboData;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    TObjectPtr<UAnimMontage> AttackMontage;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    TSubclassOf<UGameplayEffect> DamageEffect;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    float TraceRadius = 50.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Melee")
    float TraceDistance = 150.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "VA|Melee")
    TSubclassOf<UCameraShakeBase> HitCameraShake;
    
    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

    UFUNCTION()
    void OnMeleeHitEvent(FGameplayEventData Payload);
    
    void PerformMeleeTrace();
    void ApplyDamageToTarget(AActor* Target);

private:
    TObjectPtr<UAnimMontage> CurrentMontage;
    float CurrentDamageMultiplier = 1.0f;
    float CurrentPlayRate = 1.0f;
    float CurrentTraceRadius = 50.0f;
    float CurrentTraceDistance = 150.0f;
    TSubclassOf<UGameplayEffect> CurrentDamageEffect;
    
    UPROPERTY()
    TSet<TObjectPtr<AActor>> HitActors;
    
    TWeakObjectPtr<UVACombatComponent> CachedCombatComp;
    
    UVACombatComponent* GetCombatComponent() const;
};