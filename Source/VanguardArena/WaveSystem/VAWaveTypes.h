#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VAWaveTypes.generated.h"

USTRUCT(BlueprintType)
struct FVAWaveEnemyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ACharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnDelay = 0.0f;
};

USTRUCT(BlueprintType)
struct FVAWaveConfig : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WaveNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FVAWaveEnemyEntry> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsBossWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RewardAmount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TimeBetweenWaves = 5.0f;
};