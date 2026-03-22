// BTService_BossPhaseManager.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Characters/VABossCharacter.h"
#include "BTService_BossPhaseManager.generated.h"

UCLASS(DisplayName = "VA: Boss Phase Manager")
class VANGUARDARENA_API UBTService_BossPhaseManager : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_BossPhaseManager();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector CurrentPhaseKey;

	UPROPERTY(EditAnywhere, Category = "VA|Blackboard")
	FBlackboardKeySelector HealthPercentKey;
};