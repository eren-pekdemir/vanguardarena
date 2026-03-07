#include "Combat/VACombatComponent.h"

UVACombatComponent::UVACombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVACombatComponent::OpenComboWindow()
{
	bComboWindowOpen = true;

	UE_LOG(LogTemp, Verbose, TEXT("CombatComp: Combo Window AÇILDI (Index: %d)"), ComboIndex);
}

void UVACombatComponent::CloseComboWindow()
{
	bComboWindowOpen = false;

	UE_LOG(LogTemp, Verbose, TEXT("CombatComp: Combo Window KAPANDI (Index: %d)"), ComboIndex);
}

void UVACombatComponent::RequestCombo()
{

	if (bComboWindowOpen)
	{
		bWantsCombo = true;
		UE_LOG(LogTemp, Log, TEXT("CombatComp: Combo REQUEST kabul edildi! Sonraki: %d"), ComboIndex + 1);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("CombatComp: Combo REQUEST REDDEDİLDİ — window kapalı"));
	}
}

void UVACombatComponent::AdvanceCombo()
{
	// Combo index'i artır
	ComboIndex++;
	
	if (ComboIndex >= MaxComboCount)
	{
		UE_LOG(LogTemp, Log, TEXT("CombatComp: Combo CHAIN TAMAMLANDI! Sıfırlanıyor."));
		ResetCombo();
	}
	else
	{
		bWantsCombo = false;
		bComboWindowOpen = false;
		UE_LOG(LogTemp, Log, TEXT("CombatComp: Combo İLERLEDİ → Index: %d"), ComboIndex);
	}
}

void UVACombatComponent::ResetCombo()
{
	ComboIndex = 0;
	bComboWindowOpen = false;
	bWantsCombo = false;
	bIsAttacking = false;

	UE_LOG(LogTemp, Log, TEXT("CombatComp: Combo SIFIRLANDI"));
}