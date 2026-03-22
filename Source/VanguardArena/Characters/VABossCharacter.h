// VABossCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "Characters/VAEnemyCharacter.h"
#include "VABossCharacter.generated.h"

// Boss phase enum
UENUM(BlueprintType)
enum class EVABossPhase : uint8
{
	Phase1_Melee    UMETA(DisplayName = "Phase 1: Melee"),
	Phase2_AOE      UMETA(DisplayName = "Phase 2: AOE"),
	Phase3_Enraged  UMETA(DisplayName = "Phase 3: Enraged"),
	Dead            UMETA(DisplayName = "Dead")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, EVABossPhase, NewPhase);

UCLASS()
class VANGUARDARENA_API AVABossCharacter : public AVAEnemyCharacter
{
	GENERATED_BODY()

public:
	AVABossCharacter();

	// Mevcut phase
	UPROPERTY(BlueprintReadOnly, Category = "VA|Boss")
	EVABossPhase CurrentPhase = EVABossPhase::Phase1_Melee;

	// Phase değiştiğinde (UI boss bar phase göstergesi için)
	UPROPERTY(BlueprintAssignable, Category = "VA|Boss")
	FOnBossPhaseChanged OnBossPhaseChanged;

	// Phase değiştir
	UFUNCTION(BlueprintCallable, Category = "VA|Boss")
	void SetPhase(EVABossPhase NewPhase);

	// Boss adı (UI'da gösterilir)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Boss")
	FText BossName = FText::FromString(TEXT("SEVAROG"));

	// ─── PHASE AYARLARI ───

	// Phase 2'ye geçiş HP eşiği (0-1 arası yüzde)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Boss|Phase")
	float Phase2Threshold = 0.6f;

	// Phase 3'e geçiş HP eşiği
	UPROPERTY(EditDefaultsOnly, Category = "VA|Boss|Phase")
	float Phase3Threshold = 0.3f;

	// Phase 2 buff GE
	UPROPERTY(EditDefaultsOnly, Category = "VA|Boss|Phase")
	TSubclassOf<UGameplayEffect> Phase2BuffEffect;

	// Phase 3 buff GE
	UPROPERTY(EditDefaultsOnly, Category = "VA|Boss|Phase")
	TSubclassOf<UGameplayEffect> Phase3BuffEffect;

	// Phase geçiş montage'ı
	UPROPERTY(EditDefaultsOnly, Category = "VA|Boss|Phase")
	TObjectPtr<UAnimMontage> PhaseTransitionMontage;

	// Minion spawn sınıfı (Phase 2-3'te çağrılır)
	UPROPERTY(EditDefaultsOnly, Category = "VA|Boss|Phase")
	TSubclassOf<ACharacter> MinionClass;

	// Minion spawn sayısı
	UPROPERTY(EditDefaultsOnly, Category = "VA|Boss|Phase")
	int32 MinionSpawnCount = 3;

	// ─── FONKSİYONLAR ───

	// Minion çağır
	UFUNCTION(BlueprintCallable, Category = "VA|Boss")
	void SpawnMinions();

	// Override death — boss özel ölüm
	virtual void HandleDeath() override;

protected:
	virtual void BeginPlay() override;

private:
	// Phase geçişinde invincible (geçiş animasyonu sırasında hasar almaz)
	bool bIsTransitioning = false;
};