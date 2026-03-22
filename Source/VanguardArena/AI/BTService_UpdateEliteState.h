// BTService_UpdateEliteState.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateEliteState.generated.h"

UCLASS(DisplayName = "VA: Update Elite State")
class VANGUARDARENA_API UBTService_UpdateEliteState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateEliteState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector HealthPercentKey;

	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector IsTargetFacingMeKey;
};