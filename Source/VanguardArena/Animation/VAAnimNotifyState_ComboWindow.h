// VAAnimNotifyState_ComboWindow.h
// Animasyon timeline'ında combo window açılıp kapanma aralığı

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "VAAnimNotifyState_ComboWindow.generated.h"

/**
 * Montage timeline'ına yerleştirilir.
 * NotifyBegin → Combo window AÇILIR
 * NotifyEnd → Combo window KAPANIR
 * 
 * Bu aralıkta oyuncu tıklarsa combo ilerler.
 */
UCLASS(DisplayName = "VA Combo Window")
class VANGUARDARENA_API UVAAnimNotifyState_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// Notify başlangıcı — combo window açılır
	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;

	// Notify bitişi — combo window kapanır
	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	// Editörde görünen isim
	virtual FString GetNotifyName_Implementation() const override;
};