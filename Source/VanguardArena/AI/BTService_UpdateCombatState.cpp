// BTService_UpdateCombatState.cpp
#include "AI/BTService_UpdateCombatState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "VAGameplayTags.h"

UBTService_UpdateCombatState::UBTService_UpdateCombatState()
{
	NodeName = "Update Combat State";

	// Her 0.5 saniyede bir guncelle
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_UpdateCombatState::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!BBComp || !AIC) return;

	APawn* OwnerPawn = AIC->GetPawn();
	if (!OwnerPawn) return;

	// Mevcut hedefi oku
	AActor* Target = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (!Target)
	{
		// Hedef yok — degerleri sifirla
		BBComp->ClearValue(TargetLocationKey.SelectedKeyName);
		BBComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.0f);
		return;
	}

	// Hedef olu mu?
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (TargetASC && TargetASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Dead))
	{
		// Hedef oldu — blackboard temizle
		BBComp->ClearValue(TargetActorKey.SelectedKeyName);
		BBComp->ClearValue(TargetLocationKey.SelectedKeyName);
		BBComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.0f);
		return;
	}

	// Hedef konumunu guncelle
	FVector TargetLocation = Target->GetActorLocation();
	BBComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);

	// Mesafe hesapla
	float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), TargetLocation);
	BBComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);
}
