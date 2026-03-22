// VAGA_ChannelAbility.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_ChannelAbility.generated.h"

class UAnimMontage;
class UGameplayEffect;

UCLASS()
class VANGUARDARENA_API UVAGA_ChannelAbility : public UVAGameplayAbility
{
	GENERATED_BODY()

public:
	UVAGA_ChannelAbility();
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

	// Kanal sırasında periyodik olarak uygulanan effect
	UPROPERTY(EditDefaultsOnly, Category = "VA|Channel")
	TSubclassOf<UGameplayEffect> ChannelEffect;

	// Kanal animasyonu (looping)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Channel")
	TObjectPtr<UAnimMontage> ChannelMontage;

	// Maksimum kanal süresi
	UPROPERTY(EditDefaultsOnly, Category = "VA|Channel")
	float MaxChannelTime = 5.0f;

	// Periyodik uygulama aralığı (saniye)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Channel")
	float TickInterval = 0.5f;

	// AOE mi? (true = etraftaki düşmanlara, false = kendine)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Channel")
	bool bIsAOE = false;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Channel")
	float AOERadius = 500.0f;

private:
	FTimerHandle ChannelTickTimer;
	FTimerHandle MaxChannelTimer;
	bool bChannelEnding = false;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	void ChannelTick();
};