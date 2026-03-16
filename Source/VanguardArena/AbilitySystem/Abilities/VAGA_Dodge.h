// VAGA_Dodge.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_Dodge.generated.h"

class UAnimMontage;

UCLASS()
class VANGUARDARENA_API UVAGA_Dodge : public UVAGameplayAbility
{
	GENERATED_BODY()

public:
	UVAGA_Dodge();

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

	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	// Dodge hızı (cm/s)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	float DodgeSpeed = 1500.0f;

	// Dodge süresi (saniye)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	float DodgeDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	float PlayRate = 1.2f;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	FVector CalculateDodgeDirection() const;
	void DodgeMovementTick();

	FVector SavedDodgeDirection = FVector::ZeroVector;
	FTimerHandle DodgeMovementTimer;
	float DodgeElapsed = 0.0f;
};