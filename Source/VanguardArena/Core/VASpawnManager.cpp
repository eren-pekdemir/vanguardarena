// VASpawnManager.cpp
#include "Core/VASpawnManager.h"
#include "Characters/VAEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"

AVASpawnManager::AVASpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVASpawnManager::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnEnemies();
	}
}

void AVASpawnManager::SpawnEnemies()
{
	if (!EnemyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: EnemyClass atanmamış!"));
		return;
	}

	SpawnedCount = 0;
	AliveEnemies.Empty();

	// Timer ile aralıklı spawn
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &AVASpawnManager::SpawnSingleEnemy);

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle, TimerDel, SpawnInterval, true); // true = looping

	UE_LOG(LogTemp, Log, TEXT("SpawnManager: %d düşman spawn başlıyor (%.1fs aralıkla)"),
		SpawnCount, SpawnInterval);
}

void AVASpawnManager::SpawnSingleEnemy()
{
	if (SpawnedCount >= SpawnCount)
	{
		// Yeterli düşman spawn edildi — timer durdur
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("SpawnManager: Tüm düşmanlar spawn edildi (%d)"), SpawnedCount);
		return;
	}

	FTransform SpawnTransform = GetRandomSpawnTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		EnemyClass, SpawnTransform, SpawnParams);

	if (SpawnedActor)
	{
		AliveEnemies.Add(SpawnedActor);
		SpawnedCount++;

		UE_LOG(LogTemp, Log, TEXT("SpawnManager: Düşman #%d spawn edildi → %s"),
			SpawnedCount, *SpawnedActor->GetName());
	}
}

void AVASpawnManager::OnEnemyDied(AActor* DeadEnemy)
{
	if (!DeadEnemy) return;

	// Listeden çıkar
	AliveEnemies.RemoveAll([DeadEnemy](const TWeakObjectPtr<AActor>& Elem)
	{
		return !Elem.IsValid() || Elem.Get() == DeadEnemy;
	});

	// Geçersiz referansları da temizle
	AliveEnemies.RemoveAll([](const TWeakObjectPtr<AActor>& Elem)
	{
		return !Elem.IsValid();
	});

	int32 Remaining = AliveEnemies.Num();
	UE_LOG(LogTemp, Log, TEXT("SpawnManager: Düşman öldü — kalan: %d"), Remaining);

	// Tümü öldü mü?
	if (Remaining <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: TÜM DÜŞMANLAR ÖLDÜ!"));
		OnAllEnemiesDead.Broadcast();
	}
}

FTransform AVASpawnManager::GetRandomSpawnTransform() const
{
	// Level'daki tüm TargetPoint'leri bul (spawn noktaları olarak kullanılacak)
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), SpawnPoints);

	if (SpawnPoints.Num() > 0)
	{
		// Rastgele bir spawn noktası seç
		int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		return SpawnPoints[RandomIndex]->GetActorTransform();
	}

	// Spawn point yoksa → SpawnManager'ın kendi konumu etrafında rastgele
	FVector RandomOffset = FVector(
		FMath::RandRange(-500.0f, 500.0f),
		FMath::RandRange(-500.0f, 500.0f),
		0.0f
	);

	FTransform RandomTransform;
	RandomTransform.SetLocation(GetActorLocation() + RandomOffset);
	RandomTransform.SetRotation(FQuat::Identity);

	// NavMesh üzerinde geçerli nokta bul
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(RandomTransform.GetLocation(), NavLocation, FVector(500, 500, 200)))
		{
			RandomTransform.SetLocation(NavLocation.Location);
		}
	}

	return RandomTransform;
}