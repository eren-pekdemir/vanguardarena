// BTTask_ActivateGameplayAbility.cpp
#include "AI/BTTask_ActivateGameplayAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_ActivateGameplayAbility::UBTTask_ActivateGameplayAbility()
{
	NodeName = "Activate Gameplay Ability";

	// Latent task — ability bitene kadar bekleyebilir
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true; // Her AI icin ayri instance
}

EBTNodeResult::Type UBTTask_ActivateGameplayAbility::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// AI Controller'dan Pawn al
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	// ASC bul
	UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC) return EBTNodeResult::Failed;

	// Tag ile ability bul ve aktive et
	if (!AbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_ActivateAbility: AbilityTag gecersiz!"));
		return EBTNodeResult::Failed;
	}

	// AbilityTag'e sahip ability'yi bul
	bool bActivated = false;
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag))
		{
			bActivated = ASC->TryActivateAbility(Spec.Handle);
			break;
		}
	}

	if (!bActivated)
	{
		UE_LOG(LogTemp, Verbose, TEXT("BTTask_ActivateAbility: Ability aktive edilemedi (Tag: %s)"),
			*AbilityTag.ToString());
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("BTTask_ActivateAbility: %s AKTIVE EDILDI"), *AbilityTag.ToString());

	// Bekleme istemiyorsa hemen basarili don
	if (!bWaitForAbilityEnd)
	{
		return EBTNodeResult::Succeeded;
	}

	// Ability bittiginde bildirim al
	AbilityEndedDelegateHandle = ASC->OnAbilityEnded.AddUObject(
		this, &UBTTask_ActivateGameplayAbility::OnAbilityEnded, &OwnerComp);

	// InProgress — ability bitene kadar bekle
	return EBTNodeResult::InProgress;
}

void UBTTask_ActivateGameplayAbility::OnAbilityEnded(
	const FAbilityEndedData& AbilityEndedData, UBehaviorTreeComponent* OwnerComp)
{
	if (!OwnerComp) return;

	// Biten ability bizim tag'imizle eslesiyor mu?
	if (AbilityEndedData.AbilityThatEnded &&
		AbilityEndedData.AbilityThatEnded->AbilityTags.HasTag(AbilityTag))
	{
		UE_LOG(LogTemp, Log, TEXT("BTTask_ActivateAbility: %s BITTI"), *AbilityTag.ToString());

		// Delegate temizle
		APawn* Pawn = OwnerComp->GetAIOwner() ? OwnerComp->GetAIOwner()->GetPawn() : nullptr;
		if (Pawn)
		{
			UAbilitySystemComponent* ASC =
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
			if (ASC)
			{
				ASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
			}
		}

		// Task basarili
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_ActivateGameplayAbility::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// Cleanup — delegate'i kaldir
	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (Pawn)
	{
		UAbilitySystemComponent* ASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
		if (ASC)
		{
			ASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		}
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_ActivateGameplayAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Activate: %s\nWait: %s"),
		*AbilityTag.ToString(),
		bWaitForAbilityEnd ? TEXT("Yes") : TEXT("No"));
}
