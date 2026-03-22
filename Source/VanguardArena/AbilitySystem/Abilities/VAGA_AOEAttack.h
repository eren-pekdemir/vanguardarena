// VAGA_AOEAttack.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_AOEAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;

UCLASS()
class VANGUARDARENA_API UVAGA_AOEAttack : public UVAGameplayAbility
{
	GENERATED_BODY()

public:
	UVAGA_AOEAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "VA|AOE")
	TObjectPtr<UAnimMontage> AOEMontage;

	UPROPERTY(EditDefaultsOnly, Category = "VA|AOE")
	TSubclassOf<UGameplayEffect> AOEDamageEffect;

	// Ek effect'ler (DOT, slow, stun vs.)
	UPROPERTY(EditDefaultsOnly, Category = "VA|AOE")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalAOEEffects;

	UPROPERTY(EditDefaultsOnly, Category = "VA|AOE")
	float AOERadius = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|AOE")
	float PlayRate = 1.0f;

	// AOE merkezi offset (karaktere göre)
	UPROPERTY(EditDefaultsOnly, Category = "VA|AOE")
	FVector AOECenterOffset = FVector(0, 0, 0);

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnAOEEvent(FGameplayEventData Payload);

	void PerformAOE();
};