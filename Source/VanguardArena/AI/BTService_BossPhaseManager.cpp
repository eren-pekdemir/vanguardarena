// BTService_BossPhaseManager.cpp
#include "AI/BTService_BossPhaseManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/VAAttributeSet.h"
#include "Characters/VABossCharacter.h"

UBTService_BossPhaseManager::UBTService_BossPhaseManager()
{
	NodeName = "Boss Phase Manager";
	Interval = 0.5f;
	RandomDeviation = 0.05f;
}

void UBTService_BossPhaseManager::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return;

	AVABossCharacter* Boss = Cast<AVABossCharacter>(AIC->GetPawn());
	if (!Boss) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	// ─── HP YÜZDE HESAPLA ───
	UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Boss);
	if (!ASC) return;

	const UVAAttributeSet* Attributes = ASC->GetSet<UVAAttributeSet>();
	if (!Attributes) return;

	float MaxHP = Attributes->GetMaxHealth();
	float CurrentHP = Attributes->GetHealth();
	float HPPercent = (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;

	BB->SetValueAsFloat(HealthPercentKey.SelectedKeyName, HPPercent);

	// ─── PHASE KONTROL ───
	EVABossPhase TargetPhase = Boss->CurrentPhase;

	if (CurrentHP <= 0.0f)
	{
		TargetPhase = EVABossPhase::Dead;
	}
	else if (HPPercent <= Boss->Phase3Threshold)
	{
		TargetPhase = EVABossPhase::Phase3_Enraged;
	}
	else if (HPPercent <= Boss->Phase2Threshold)
	{
		TargetPhase = EVABossPhase::Phase2_AOE;
	}
	else
	{
		TargetPhase = EVABossPhase::Phase1_Melee;
	}

	// Phase değişti mi?
	if (TargetPhase != Boss->CurrentPhase)
	{
		Boss->SetPhase(TargetPhase);
	}

	// Blackboard'a yaz
	BB->SetValueAsInt(CurrentPhaseKey.SelectedKeyName, (int32)Boss->CurrentPhase);
}