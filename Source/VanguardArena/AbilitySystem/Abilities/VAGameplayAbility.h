#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "VAGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class EVAAbilityActivationPolicy : uint8
{
    OnInputTriggered  UMETA(DisplayName = "On Input Triggered"),
    
    OnSpawn           UMETA(DisplayName = "On Spawn"),
    
    OnGiven           UMETA(DisplayName = "On Given")
};


UCLASS(Abstract)
class VANGUARDARENA_API UVAGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UVAGameplayAbility();
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Input")
    FGameplayTag InputTag;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Activation")
    EVAAbilityActivationPolicy ActivationPolicy = EVAAbilityActivationPolicy::OnInputTriggered;

   
    EVAAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
    
    UFUNCTION(BlueprintCallable, Category = "VA|Damage")
    void ApplyAOEDamage(
        const FVector& Center,
        float Radius,
        TSubclassOf<UGameplayEffect> EffectClass,
        const TArray<AActor*>& IgnoreActors);

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
    
    UFUNCTION(BlueprintCallable, Category = "VA|Ability")
    ACharacter* GetAvatarCharacter() const;
    
    UFUNCTION(BlueprintCallable, Category = "VA|Ability")
    UAbilitySystemComponent* GetAvatarASC() const;
    
    UPROPERTY(EditDefaultsOnly, Category = "VA|Effects")
    TArray<TSubclassOf<UGameplayEffect>> AdditionalEffectsOnHit;
    
    // Vuruş partikülü (Cascade)
    UPROPERTY(EditDefaultsOnly, Category = "VA|VFX")
    TObjectPtr<UParticleSystem> HitParticle;

    // Vuruş sesi
    UPROPERTY(EditDefaultsOnly, Category = "VA|VFX")
    TObjectPtr<USoundBase> HitSound;

    // Camera Shake
    UPROPERTY(EditDefaultsOnly, Category = "VA|VFX")
    TSubclassOf<UCameraShakeBase> HitCameraShake;

    // Helper fonksiyon — herhangi bir ability çağırabilir
    UFUNCTION(BlueprintCallable, Category = "VA|VFX")
    void PlayHitEffects(const FVector& Location, const FVector& Normal);
};