// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VAPlayerController.generated.h"

// Forward declaration
class UInputMappingContext;

UCLASS()
class VANGUARDARENA_API AVAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVAPlayerController();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "VA|Input")
	TObjectPtr<UInputMappingContext> CombatMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "VA|Input")
	int32 CombatMappingPriority = 0;
};