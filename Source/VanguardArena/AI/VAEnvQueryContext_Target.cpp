// VAEnvQueryContext_Target.cpp
#include "AI/VAEnvQueryContext_Target.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UVAEnvQueryContext_Target::ProvideContext(
	FEnvQueryInstance& QueryInstance,
	FEnvQueryContextData& ContextData) const
{
	// EQS'i çalıştıran AI'ın controller'ından Blackboard'daki hedefi al
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (!QueryOwner) return;

	APawn* Pawn = Cast<APawn>(QueryOwner);
	if (!Pawn) return;

	AAIController* AIC = Cast<AAIController>(Pawn->GetController());
	if (!AIC) return;

	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (Target)
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
	}
}