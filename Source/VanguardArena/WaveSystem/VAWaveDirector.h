// VAWaveDirector.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveSystem/VAWaveTypes.h"
#include "GameplayEffect.h"
#include "VAWaveDirector.generated.h"

class ACharacter;
class UDataTable;

// ─── DELEGATE'LER (UI bağlantısı) ───
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveCompleted, int32, WaveNumber, int32, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyCountChanged, int32, Remaining, int32, Total);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, float, TimeRemaining);

UCLASS()
class VANGUARDARENA_API AVAWaveDirector : public AActor
{
	GENERATED_BODY()

public:
	AVAWaveDirector();

	// ─── KONTROL FONKSİYONLARI ───

	// Wave sistemini başlat
	UFUNCTION(BlueprintCallable, Category = "VA|Wave")
	void StartWaveSystem();

	// Mevcut wave'i başlat
	UFUNCTION(BlueprintCallable, Category = "VA|Wave")
	void StartCurrentWave();

	// Düşman öldüğünde çağrılır
	UFUNCTION()
	void OnEnemyDied(AActor* DeadEnemy);

	// ─── DELEGATE'LER ───

	UPROPERTY(BlueprintAssignable, Category = "VA|Wave")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "VA|Wave")
	FOnWaveCompleted OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "VA|Wave")
	FOnAllWavesCompleted OnAllWavesCompleted;

	UPROPERTY(BlueprintAssignable, Category = "VA|Wave")
	FOnEnemyCountChanged OnEnemyCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "VA|Wave")
	FOnCountdownTick OnCountdownTick;

	// ─── AYARLAR ───

	// Wave konfigürasyon tablosu
	UPROPERTY(EditAnywhere, Category = "VA|Wave")
	TObjectPtr<UDataTable> WaveDataTable;

	// BeginPlay'de otomatik başla mı?
	UPROPERTY(EditAnywhere, Category = "VA|Wave")
	bool bAutoStart = true;

	// DataTable bittikten sonra sonsuz mod mu?
	UPROPERTY(EditAnywhere, Category = "VA|Wave")
	bool bEndlessMode = true;

	// Sonsuz modda difficulty çarpanı artışı (her wave)
	UPROPERTY(EditAnywhere, Category = "VA|Wave|Difficulty")
	float DifficultyIncrement = 0.15f;

	// ─── OKUMA ───

	UPROPERTY(BlueprintReadOnly, Category = "VA|Wave")
	int32 CurrentWaveIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VA|Wave")
	int32 TotalWaves = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VA|Wave")
	int32 AliveEnemyCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VA|Wave")
	int32 TotalEnemiesThisWave = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VA|Wave")
	float DifficultyMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "VA|Wave")
	bool bWaveInProgress = false;

protected:
	virtual void BeginPlay() override;

private:
	// Wave config'lerini DataTable'dan oku
	void LoadWaveConfigs();

	// Tek düşman spawn et
	void SpawnEnemy(TSubclassOf<ACharacter> EnemyClass);

	// Wave spawn timer
	void ProcessSpawnQueue();

	// Wave tamamlanma kontrolü
	void CheckWaveComplete();

	// Sonraki wave'e geçiş countdown'u
	void StartCountdown(float Duration);
	void CountdownTick();

	// Spawn noktası bul
	FTransform GetRandomSpawnTransform() const;

	// Difficulty uygula (düşman stat'larına çarpan)
	void ApplyDifficultyToEnemy(AActor* Enemy);

	// Veriler
	TArray<FVAWaveConfig> WaveConfigs;
	TArray<TWeakObjectPtr<AActor>> AliveEnemies;

	// Spawn kuyruğu
	struct FSpawnQueueItem
	{
		TSubclassOf<ACharacter> EnemyClass;
		float SpawnTime; // wave başlangıcından itibaren
	};
	TArray<FSpawnQueueItem> SpawnQueue;
	int32 SpawnQueueIndex = 0;
	float WaveStartTime = 0.0f;

	// Timer'lar
	FTimerHandle SpawnTimerHandle;
	FTimerHandle CountdownTimerHandle;
	float CountdownRemaining = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "VA|Wave|Difficulty")
	TSubclassOf<UGameplayEffect> DifficultyScaleEffect;
};