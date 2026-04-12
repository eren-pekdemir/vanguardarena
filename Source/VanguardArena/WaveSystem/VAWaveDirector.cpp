// VAWaveDirector.cpp
#include "WaveSystem/VAWaveDirector.h"
#include "Characters/VAEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/VAAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"

AVAWaveDirector::AVAWaveDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVAWaveDirector::BeginPlay()
{
	Super::BeginPlay();
	LoadWaveConfigs();

	if (bAutoStart)
	{
		// İlk wave'den önce kısa bekleme
		StartCountdown(3.0f);
	}
}

void AVAWaveDirector::LoadWaveConfigs()
{
	WaveConfigs.Empty();

	if (!WaveDataTable)
	{
		return;
	}

	TArray<FName> RowNames = WaveDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FVAWaveConfig* Row = WaveDataTable->FindRow<FVAWaveConfig>(RowName, TEXT(""));
		if (Row)
		{
			WaveConfigs.Add(*Row);
		}
	}

	// Wave numarasına göre sırala
	WaveConfigs.Sort([](const FVAWaveConfig& A, const FVAWaveConfig& B)
	{
		return A.WaveNumber < B.WaveNumber;
	});

	TotalWaves = WaveConfigs.Num();
}

void AVAWaveDirector::StartWaveSystem()
{
	CurrentWaveIndex = 0;
	DifficultyMultiplier = 1.0f;
	StartCurrentWave();
}

void AVAWaveDirector::StartCurrentWave()
{
	if (WaveConfigs.Num() == 0)
	{
		return;
	}

	// Mevcut wave config'i al
	FVAWaveConfig CurrentConfig;

	if (CurrentWaveIndex < WaveConfigs.Num())
	{
		// DataTable'dan oku
		CurrentConfig = WaveConfigs[CurrentWaveIndex];
	}
	else if (bEndlessMode)
	{
		// Endless mod — son wave'i tekrarla, difficulty artır
		CurrentConfig = WaveConfigs.Last();
		CurrentConfig.WaveNumber = CurrentWaveIndex + 1;
		DifficultyMultiplier += DifficultyIncrement;
	}
	else
	{
		// Tüm wave'ler bitti
		OnAllWavesCompleted.Broadcast();
		return;
	}

	// State sıfırla
	bWaveInProgress = true;
	AliveEnemies.Empty();
	AliveEnemyCount = 0;
	TotalEnemiesThisWave = 0;
	SpawnQueue.Empty();
	SpawnQueueIndex = 0;
	WaveStartTime = GetWorld()->GetTimeSeconds();

	// Spawn kuyruğu oluştur
	for (const FVAWaveEnemyEntry& Entry : CurrentConfig.Enemies)
	{
		if (!Entry.EnemyClass) continue;

		for (int32 i = 0; i < Entry.Count; i++)
		{
			FSpawnQueueItem Item;
			Item.EnemyClass = Entry.EnemyClass;
			Item.SpawnTime = Entry.SpawnDelay + (i * CurrentConfig.SpawnInterval);
			SpawnQueue.Add(Item);
			TotalEnemiesThisWave++;
		}
	}

	// Spawn zamanına göre sırala
	SpawnQueue.Sort([](const FSpawnQueueItem& A, const FSpawnQueueItem& B)
	{
		return A.SpawnTime < B.SpawnTime;
	});


	// Delegate
	OnWaveStarted.Broadcast(CurrentConfig.WaveNumber);
	OnEnemyCountChanged.Broadcast(TotalEnemiesThisWave, TotalEnemiesThisWave);

	// Spawn timer başlat
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this, &AVAWaveDirector::ProcessSpawnQueue,
		0.1f, true); // 100ms'de bir kontrol
}

void AVAWaveDirector::ProcessSpawnQueue()
{
	if (SpawnQueueIndex >= SpawnQueue.Num())
	{
		// Tüm düşmanlar spawn edildi — timer durdur
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	float ElapsedTime = GetWorld()->GetTimeSeconds() - WaveStartTime;

	// Zamanı gelen düşmanları spawn et
	while (SpawnQueueIndex < SpawnQueue.Num() &&
		   SpawnQueue[SpawnQueueIndex].SpawnTime <= ElapsedTime)
	{
		SpawnEnemy(SpawnQueue[SpawnQueueIndex].EnemyClass);
		SpawnQueueIndex++;
	}
}

void AVAWaveDirector::SpawnEnemy(TSubclassOf<ACharacter> EnemyClass)
{
	if (!EnemyClass) return;

	FTransform SpawnTransform = GetRandomSpawnTransform();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(
		EnemyClass, SpawnTransform, Params);

	if (SpawnedEnemy)
	{
		AliveEnemies.Add(SpawnedEnemy);
		AliveEnemyCount = AliveEnemies.Num();

		// Difficulty scaling uygula
		ApplyDifficultyToEnemy(SpawnedEnemy);

	}
}

void AVAWaveDirector::OnEnemyDied(AActor* DeadEnemy)
{
	if (!DeadEnemy) return;

	// Listeden çıkar
	AliveEnemies.RemoveAll([DeadEnemy](const TWeakObjectPtr<AActor>& Elem)
	{
		return !Elem.IsValid() || Elem.Get() == DeadEnemy;
	});

	// Geçersiz referansları temizle
	AliveEnemies.RemoveAll([](const TWeakObjectPtr<AActor>& Elem)
	{
		return !Elem.IsValid();
	});

	AliveEnemyCount = AliveEnemies.Num();


	// UI güncelle
	OnEnemyCountChanged.Broadcast(AliveEnemyCount, TotalEnemiesThisWave);

	// Wave bitti mi?
	CheckWaveComplete();
}

void AVAWaveDirector::CheckWaveComplete()
{
	// Tüm düşmanlar spawn edildi VE hepsi öldü mü?
	if (SpawnQueueIndex >= SpawnQueue.Num() && AliveEnemyCount <= 0)
	{
		bWaveInProgress = false;

		// Mevcut wave config'den ödül al
		int32 Reward = 100;
		float WaitTime = 5.0f;

		if (CurrentWaveIndex < WaveConfigs.Num())
		{
			Reward = WaveConfigs[CurrentWaveIndex].RewardAmount;
			WaitTime = WaveConfigs[CurrentWaveIndex].TimeBetweenWaves;
		}


		OnWaveCompleted.Broadcast(CurrentWaveIndex + 1, Reward);

		// Sonraki wave'e geç
		CurrentWaveIndex++;

		// Tüm wave'ler bitti mi?
		if (CurrentWaveIndex >= WaveConfigs.Num() && !bEndlessMode)
		{
			OnAllWavesCompleted.Broadcast();
			return;
		}

		// Countdown başlat
		StartCountdown(WaitTime);
	}
}

void AVAWaveDirector::StartCountdown(float Duration)
{
	CountdownRemaining = Duration;

	GetWorld()->GetTimerManager().SetTimer(
		CountdownTimerHandle,
		this, &AVAWaveDirector::CountdownTick,
		1.0f, true); // Her saniye
}

void AVAWaveDirector::CountdownTick()
{
	CountdownRemaining -= 1.0f;

	OnCountdownTick.Broadcast(CountdownRemaining);


	if (CountdownRemaining <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
		StartCurrentWave();
	}
}

FTransform AVAWaveDirector::GetRandomSpawnTransform() const
{
	// TargetPoint'leri spawn noktası olarak kullan
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), SpawnPoints);

	if (SpawnPoints.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
		return SpawnPoints[Index]->GetActorTransform();
	}

	// Spawn point yoksa → WaveDirector etrafında rastgele
	FVector Offset(
		FMath::RandRange(-800.0f, 800.0f),
		FMath::RandRange(-800.0f, 800.0f),
		0.0f);

	FTransform T;
	T.SetLocation(GetActorLocation() + Offset);

	// NavMesh'e project et
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(T.GetLocation(), NavLoc, FVector(500, 500, 200)))
		{
			T.SetLocation(NavLoc.Location);
		}
	}

	return T;
}

void AVAWaveDirector::ApplyDifficultyToEnemy(AActor* Enemy)
{
	if (DifficultyMultiplier <= 1.0f) return;

	UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
	if (!ASC) return;

	const UVAAttributeSet* Attributes = ASC->GetSet<UVAAttributeSet>();
	if (!Attributes) return;

	// Mevcut değerleri oku ve çarpanla artır
	float NewMaxHealth = Attributes->GetMaxHealth() * DifficultyMultiplier;
	float NewAttackPower = Attributes->GetAttackPower() * DifficultyMultiplier;

	ASC->ApplyModToAttribute(UVAAttributeSet::GetMaxHealthAttribute(), EGameplayModOp::Override, NewMaxHealth);
	ASC->ApplyModToAttribute(UVAAttributeSet::GetHealthAttribute(), EGameplayModOp::Override, NewMaxHealth);
	ASC->ApplyModToAttribute(UVAAttributeSet::GetAttackPowerAttribute(), EGameplayModOp::Override, NewAttackPower);

}