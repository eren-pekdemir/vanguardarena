// VAGA_Dodge.h
// Dodge/Roll ability — i-frame ile hasar kaçırma

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
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	// ─── DODGE AYARLARI ───

	// Dodge animasyonu
	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	// Dodge mesafesi (root motion YOKSA kullanılır)
	// Root motion varsa animasyonun kendi hareketi kullanılır
	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	float DodgeDistance = 500.0f;

	// Dodge süresi (root motion yoksa LaunchCharacter ile)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	float DodgeDuration = 0.4f;

	// Root motion kullan mı?
	// TRUE → animasyonun kendi hareketi (daha doğal)
	// FALSE → LaunchCharacter ile kod bazlı hareket
	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	bool bUseRootMotion = false;

	// Montage oynatma hızı
	UPROPERTY(EditDefaultsOnly, Category = "VA|Dodge")
	float PlayRate = 1.2f;

	// ─── CALLBACKS ───

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

private:
	// Dodge yönünü hesapla
	FVector CalculateDodgeDirection() const;
};