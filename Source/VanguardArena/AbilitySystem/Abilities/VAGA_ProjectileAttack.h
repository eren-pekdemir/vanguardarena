// VAGA_ProjectileAttack.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "VAGA_ProjectileAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;
class AVAProjectile;

UCLASS()
class VANGUARDARENA_API UVAGA_ProjectileAttack : public UVAGameplayAbility
{
	GENERATED_BODY()

public:
	UVAGA_ProjectileAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	// Mermi sınıfı (Blueprint'te atanır)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	TSubclassOf<AVAProjectile> ProjectileClass;

	// Fırlatma animasyonu
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	TObjectPtr<UAnimMontage> FireMontage;

	// Mermi hasar effect'i
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// Mermi spawn offset (karakterin önünde)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	FVector SpawnOffset = FVector(100.0f, 0.0f, 50.0f);

	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	float PlayRate = 1.0f;

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	// AnimNotify event — mermi fırlatma anı
	UFUNCTION()
	void OnSpawnProjectileEvent(FGameplayEventData Payload);

	void SpawnProjectile();
};