// VAAnimNotifyState_IFrame.cpp
#include "Animation/VAAnimNotifyState_IFrame.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "VAGameplayTags.h"

void UVAAnimNotifyState_IFrame::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;

	// ASC'ye State.Invincible tag'i ekle
	UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	if (ASC)
	{
		ASC->AddLooseGameplayTag(FVAGameplayTags::Get().State_Invincible);
		UE_LOG(LogTemp, Log, TEXT("i-Frame: BAŞLADI — State.Invincible eklendi"));
	}
}

void UVAAnimNotifyState_IFrame::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation,EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;

	// State.Invincible tag'ini kaldır
	UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(FVAGameplayTags::Get().State_Invincible);
		UE_LOG(LogTemp, Log, TEXT("i-Frame: BİTTİ — State.Invincible kaldırıldı"));
	}
}


FString UVAAnimNotifyState_IFrame::GetNotifyName_Implementation() const
{
	return TEXT("VA: i-Frame (Invincible)");
}