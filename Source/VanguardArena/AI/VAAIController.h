// VAAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AIPerceptionTypes.h"
#include "VAAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class VANGUARDARENA_API AVAAIController : public AAIController
{
	GENERATED_BODY()

public:
	AVAAIController();

	// Pawn possess edildiğinde çağrılır
	virtual void OnPossess(APawn* InPawn) override;

	// Pawn unpossess edildiğinde
	virtual void OnUnPossess() override;

	// ─── BLACKBOARD ERIŞIM ───

	// Blackboard'a hedef yaz
	UFUNCTION(BlueprintCallable, Category = "VA|AI")
	void SetTargetActor(AActor* Target);

	// Blackboard'dan hedef oku
	UFUNCTION(BlueprintCallable, Category = "VA|AI")
	AActor* GetTargetActor() const;

protected:
	// ─── BEHAVIOR TREE ───
	// Blueprint'te atanacak
	UPROPERTY(EditDefaultsOnly, Category = "VA|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	// ─── BLACKBOARD KEY İSİMLERİ ───
	// Blackboard'daki key isimleri — tüm BT Task/Service'ler bunları kullanır
	UPROPERTY(EditDefaultsOnly, Category = "VA|AI|Blackboard")
	FName BB_TargetActor = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, Category = "VA|AI|Blackboard")
	FName BB_TargetLocation = TEXT("TargetLocation");

	UPROPERTY(EditDefaultsOnly, Category = "VA|AI|Blackboard")
	FName BB_DistanceToTarget = TEXT("DistanceToTarget");

	UPROPERTY(EditDefaultsOnly, Category = "VA|AI|Blackboard")
	FName BB_AttackRange = TEXT("AttackRange");

	// ─── AI PERCEPTION ───
	UPROPERTY(VisibleAnywhere, Category = "VA|AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	// Sense
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
	
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	// Perception güncelleme callback
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// ─── TEAM ───
	// Düşmanlar Team 1, Oyuncu Team 0
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	

private:
	FGenericTeamId TeamId;
	
	void SetTargetInBlackboard(AActor* Target);
};