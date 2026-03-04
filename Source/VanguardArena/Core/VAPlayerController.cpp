// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/VAPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

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

			UE_LOG(LogTemp, Log, TEXT("VA: Combat IMC eklendi (Priority: %d)"), CombatMappingPriority);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VA: Combat IMC eksik! PlayerController Blueprint'te ayarla."));
		}
	}
}