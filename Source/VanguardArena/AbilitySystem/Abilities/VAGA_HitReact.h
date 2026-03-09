// VAGA_HitReact.h
// Vuruş tepkisi ability'si — yön bazlı animasyon oynatır

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_HitReact.generated.h"

class UAnimMontage;

UCLASS()
class VANGUARDARENA_API UVAGA_HitReact : public UVAGameplayAbility
{
	GENERATED_BODY()

public:
	UVAGA_HitReact();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	// ─── 4 YÖNLÜ HIT REACT MONTAGE ───
	UPROPERTY(EditDefaultsOnly, Category = "VA|HitReact")
	TObjectPtr<UAnimMontage> HitReactFront;

	UPROPERTY(EditDefaultsOnly, Category = "VA|HitReact")
	TObjectPtr<UAnimMontage> HitReactBack;

	UPROPERTY(EditDefaultsOnly, Category = "VA|HitReact")
	TObjectPtr<UAnimMontage> HitReactLeft;

	UPROPERTY(EditDefaultsOnly, Category = "VA|HitReact")
	TObjectPtr<UAnimMontage> HitReactRight;

	// Montage bittiğinde
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

private:
	// Vuruş yönüne göre doğru montage'ı seç
	UAnimMontage* SelectMontageByDirection(const FVector& HitDirection) const;
};