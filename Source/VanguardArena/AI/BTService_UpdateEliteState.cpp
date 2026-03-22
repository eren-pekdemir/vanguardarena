// BTService_UpdateEliteState.cpp
#include "AI/BTService_UpdateEliteState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/VAAttributeSet.h"
#include "GameFramework/Character.h"

UBTService_UpdateEliteState::UBTService_UpdateEliteState()
{
	NodeName = "Update Elite State";
	Interval = 0.3f;
	RandomDeviation = 0.05f;
}

void UBTService_UpdateEliteState::TickNode(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!BB || !AIC) return;

	APawn* OwnerPawn = AIC->GetPawn();
	if (!OwnerPawn) return;

	// ─── HEALTH PERCENT ───
	UAbilitySystemComponent* OwnerASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);

	if (OwnerASC)
	{
		const UVAAttributeSet* Attributes = OwnerASC->GetSet<UVAAttributeSet>();
		if (Attributes)
		{
			float MaxHP = Attributes->GetMaxHealth();
			float CurrentHP = Attributes->GetHealth();
			float Percent = (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;
			BB->SetValueAsFloat(HealthPercentKey.SelectedKeyName, Percent);
		}
	}

	// ─── IS TARGET FACING ME ───
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (Target && OwnerPawn)
	{
		FVector ToMe = (OwnerPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
		ToMe.Z = 0.0f;
		FVector TargetForward = Target->GetActorForwardVector();
		TargetForward.Z = 0.0f;

		float Dot = FVector::DotProduct(TargetForward.GetSafeNormal(), ToMe.GetSafeNormal());
		// Dot > 0.5 → hedef kabaca bana bakıyor (60° koni)
		bool bFacingMe = (Dot > 0.5f);

		BB->SetValueAsBool(IsTargetFacingMeKey.SelectedKeyName, bFacingMe);
	}
}