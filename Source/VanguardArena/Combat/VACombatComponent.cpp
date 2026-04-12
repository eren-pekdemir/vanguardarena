#include "Combat/VACombatComponent.h"

UVACombatComponent::UVACombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVACombatComponent::OpenComboWindow()
{
	bComboWindowOpen = true;

}

void UVACombatComponent::CloseComboWindow()
{
	bComboWindowOpen = false;

}

void UVACombatComponent::RequestCombo()
{

	if (bComboWindowOpen)
	{
		bWantsCombo = true;
	}
	else
	{
	}
}

void UVACombatComponent::AdvanceCombo()
{
	// Combo index'i artır
	ComboIndex++;
	
	if (ComboIndex >= MaxComboCount)
	{
		ResetCombo();
	}
	else
	{
		bWantsCombo = false;
		bComboWindowOpen = false;
	}
}

void UVACombatComponent::ResetCombo()
{
	ComboIndex = 0;
	bComboWindowOpen = false;
	bWantsCombo = false;
	bIsAttacking = false;

}