// VAAnimNotifyState_ComboWindow.cpp
#include "Animation/VAAnimNotifyState_ComboWindow.h"
#include "Combat/VACombatComponent.h"
#include "GameFramework/Character.h"

void UVAAnimNotifyState_ComboWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;

	// CombatComponent'i bul → combo window aç
	UVACombatComponent* CombatComp = Owner->FindComponentByClass<UVACombatComponent>();
	if (CombatComp)
	{
		CombatComp->OpenComboWindow();
	}
}

void UVAAnimNotifyState_ComboWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;

	UVACombatComponent* CombatComp = Owner->FindComponentByClass<UVACombatComponent>();
	if (CombatComp)
	{
		CombatComp->CloseComboWindow();
	}
}

FString UVAAnimNotifyState_ComboWindow::GetNotifyName_Implementation() const
{
	return TEXT("VA: Combo Window");
}