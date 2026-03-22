// VAProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "VAProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UGameplayEffect;

UCLASS()
class VANGUARDARENA_API AVAProjectile : public AActor
{
	GENERATED_BODY()

public:
	AVAProjectile();

	// Mermiyi fırlatan ability'nin effect context'i
	// Hasar uygulanırken source bilgisi için
	FGameplayEffectSpecHandle DamageSpecHandle;

	// Fırlatan actor (hasar kaynağı)
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

protected:
	virtual void BeginPlay() override;

	// ─── COMPONENTS ───

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VA|Projectile")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VA|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VA|Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// ─── AYARLAR ───

	// Mermi hızı (cm/s)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	float Speed = 2000.0f;

	// Mermi ömrü (saniye) — bu süre sonunda yok olur
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	float LifeTime = 5.0f;

	// Çarptığında AOE hasar mı? (true = etraftaki herkese hasar)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	bool bExplodeOnImpact = false;

	// AOE yarıçapı (bExplodeOnImpact = true ise)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Projectile")
	float ExplosionRadius = 300.0f;

	// Çarpışma callback
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	// Tek hedefe hasar uygula
	void ApplyDamageToTarget(AActor* Target);

	// AOE hasar uygula
	void ApplyAOEDamage(const FVector& Location);

	// Çift hasar engelleme
	bool bHasHit = false;
};