// VASpawnManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VASpawnManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllEnemiesDead);

UCLASS()
class VANGUARDARENA_API AVASpawnManager : public AActor
{
	GENERATED_BODY()

public:
	AVASpawnManager();

	// Spawn işlemini başlat
	UFUNCTION(BlueprintCallable, Category = "VA|Spawn")
	void SpawnEnemies();

	// Düşman öldüğünde çağrılır
	UFUNCTION()
	void OnEnemyDied(AActor* DeadEnemy);

	// Tüm düşmanlar öldüğünde
	UPROPERTY(BlueprintAssignable, Category = "VA|Spawn")
	FOnAllEnemiesDead OnAllEnemiesDead;

	// ─── AYARLAR ───

	// Spawn edilecek düşman sınıfı
	UPROPERTY(EditAnywhere, Category = "VA|Spawn")
	TSubclassOf<ACharacter> EnemyClass;

	// Kaç düşman spawn edilecek
	UPROPERTY(EditAnywhere, Category = "VA|Spawn")
	int32 SpawnCount = 5;

	// Spawn'lar arası bekleme süresi (saniye)
	UPROPERTY(EditAnywhere, Category = "VA|Spawn")
	float SpawnInterval = 1.0f;

	// BeginPlay'de otomatik spawn et mi?
	UPROPERTY(EditAnywhere, Category = "VA|Spawn")
	bool bSpawnOnBeginPlay = true;

protected:
	virtual void BeginPlay() override;

private:
	// Yaşayan düşmanlar
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> AliveEnemies;

	// Spawn edilen toplam
	int32 SpawnedCount = 0;

	// Spawn timer
	FTimerHandle SpawnTimerHandle;

	// Tek bir düşman spawn et
	void SpawnSingleEnemy();

	// Spawn noktası bul (Spawn Point actor'lerinden rastgele)
	FTransform GetRandomSpawnTransform() const;
};