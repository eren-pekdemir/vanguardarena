// VAGA_SmashAttack.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_SmashAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UCameraShakeBase;

UCLASS()
class VANGUARDARENA_API UVAGA_SmashAttack : public UVAGameplayAbility
{
	GENERATED_BODY()

public:
	UVAGA_SmashAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// Smash montage
	UPROPERTY(EditDefaultsOnly, Category = "VA|Smash")
	TObjectPtr<UAnimMontage> SmashMontage;

	// Hasar efekti
	UPROPERTY(EditDefaultsOnly, Category = "VA|Smash")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// Camera Shake
	UPROPERTY(EditDefaultsOnly, Category = "VA|Smash")
	TSubclassOf<UCameraShakeBase> SmashCameraShake;

	// AOE yarıçapı
	UPROPERTY(EditDefaultsOnly, Category = "VA|Smash")
	float SmashRadius = 400.0f;

	// Fırlatma gücü (oyuncuyu uzağa atar)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Smash")
	float LaunchStrength = 1200.0f;

	// Yukarı fırlatma gücü
	UPROPERTY(EditDefaultsOnly, Category = "VA|Smash")
	float LaunchUpStrength = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Smash")
	float PlayRate = 1.0f;

private:
	UFUNCTION()
	void OnSmashEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	void PerformSmash();
};
