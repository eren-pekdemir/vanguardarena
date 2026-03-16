// BTTask_ActivateGameplayAbility.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateGameplayAbility.generated.h"

class UAbilitySystemComponent;

UCLASS(DisplayName = "VA: Activate Gameplay Ability")
class VANGUARDARENA_API UBTTask_ActivateGameplayAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateGameplayAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Aktive edilecek ability'nin tag'i
	// Ornek: Ability.Attack.Light veya Ability.Attack.Heavy
	UPROPERTY(EditAnywhere, Category = "VA|Ability")
	FGameplayTag AbilityTag;

	// Ability bitene kadar bekle mi?
	// TRUE -> ability bitince task tamamlanir
	// FALSE -> ability baslatilir, hemen Succeeded doner
	UPROPERTY(EditAnywhere, Category = "VA|Ability")
	bool bWaitForAbilityEnd = true;

private:
	// Ability bitti callback
	void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData, UBehaviorTreeComponent* OwnerComp);

	// Delegate handle (cleanup icin)
	FDelegateHandle AbilityEndedDelegateHandle;
};
