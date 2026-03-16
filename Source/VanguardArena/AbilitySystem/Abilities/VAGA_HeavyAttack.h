// VAGA_HeavyAttack.h
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
	void OnInputReleased();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	FName WindUpSectionName = TEXT("WindUp");

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	FName ReleaseSectionName = TEXT("Release");

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float TraceRadius = 75.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float TraceDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float MinChargeTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float MaxChargeTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float FullChargeThreshold = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float Level1Multiplier = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Heavy")
	float Level2Multiplier = 3.0f;

private:
	float ChargeStartTime = 0.0f;
	float CurrentDamageMultiplier = 1.0f;
	bool bHasReleased = false;

	FTimerHandle MaxChargeTimerHandle;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActors;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMeleeHitEvent(FGameplayEventData Payload);

	void PerformRelease();
	void PerformTrace();
	void ApplyDamageToTarget(AActor* Target);
	void Cleanup();
};