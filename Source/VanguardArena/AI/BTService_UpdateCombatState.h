// BTService_UpdateCombatState.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateCombatState.generated.h"

UCLASS(DisplayName = "VA: Update Combat State")
class VANGUARDARENA_API UBTService_UpdateCombatState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateCombatState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Blackboard key'leri (editorde secilebilir)
	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;
};
