// VAGA_HeavyAttack.h
// Hold-Release charge mechanic heavy attack
// Tek montage, iki section: Charge (loop) → Release (vuruş)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_HeavyAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;

UCLASS()
class VANGUARDARENA_API UVAGA_HeavyAttack : public UVAGameplayAbility
{
	GENERATED_BODY()

public:
	UVAGA_HeavyAttack();

	// Input release — VACharacterBase çağırır
	void OnInputReleased();

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

	// ─── MONTAGE ───

	// Tek montage — içinde "Charge" ve "Release" section'ları var
	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	// Section isimleri (montage editöründeki isimlerle eşleşmeli)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	FName ChargeSectionName = TEXT("Charge");

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	FName ReleaseSectionName = TEXT("Release");

	// ─── HASAR ───

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float TraceRadius = 75.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float TraceDistance = 200.0f;

	// ─── CHARGE AYARLARI ───

	// Bu süreden kısa basarsan vuruş iptal olur
	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float MinChargeTime = 0.3f;

	// Bu süreye ulaşırsan otomatik release
	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float MaxChargeTime = 2.0f;

	// Full charge eşiği (bu sürenin üstü = Level 2)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float FullChargeThreshold = 1.0f;

	// Level 1 hasar çarpanı (MinChargeTime - FullChargeThreshold arası)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float Level1Multiplier = 2.0f;

	// Level 2 hasar çarpanı (FullChargeThreshold üstü)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float Level2Multiplier = 3.0f;

	// ─── CALLBACKS ───

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMeleeHitEvent(FGameplayEventData Payload);

private:
	float ChargeStartTime = 0.0f;
	float CurrentDamageMultiplier = 1.0f;
	bool bHasReleased = false;
	FTimerHandle MaxChargeTimerHandle;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;

	void PerformRelease();
	void PerformTrace();
	void ApplyDamageToTarget(AActor* Target);
	void OnMaxChargeReached();
};