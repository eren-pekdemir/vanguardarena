// VAAnimNotifyState_IFrame.h
// Dodge animasyonunda hasar bağışıklık penceresi

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "VAAnimNotifyState_IFrame.generated.h"

UCLASS(DisplayName = "VA i-Frame (Invincible)")
class VANGUARDARENA_API UVAAnimNotifyState_IFrame : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;

	virtual void NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};