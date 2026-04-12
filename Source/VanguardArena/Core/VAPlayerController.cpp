// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/VAPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"

AVAPlayerController::AVAPlayerController()
{
	bReplicates = true;
}

void AVAPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

		if (InputSubsystem && CombatMappingContext)
		{
			InputSubsystem->AddMappingContext(CombatMappingContext, CombatMappingPriority);

		}
		else
		{
		}
		
		
	}
	
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

void AVAPlayerController::UpdateHUDHealth(float CurrentHealth, float MaxHealth)
{
	if (!HUDWidget) return;

	// Blueprint'teki UpdateHealth fonksiyonunu çağır
	UFunction* Func = HUDWidget->FindFunction(TEXT("UpdateHealth"));
	if (Func)
	{
		struct
		{
			float Current;
			float Max;
		} Params;
		Params.Current = CurrentHealth;
		Params.Max = MaxHealth;
		HUDWidget->ProcessEvent(Func, &Params);
	}
}