// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h" 
#include "AbilitySystem/VAAttributeSet.h"
#include "AbilitySystem/Abilities/VAGameplayAbility.h"
#include "Input/VAInputConfig.h"
#include "AbilitySystem/VAAbilitySystemComponent.h"
#include "Combat/VACombatComponent.h"
#include "Combat\VATargetLockComponent.h"
#include "GenericTeamAgentInterface.h"
#include "VACharacterBase.generated.h"

struct FInputActionValue;

UCLASS(Abstract)
class VANGUARDARENA_API AVACharacterBase : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AVACharacterBase();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPROPERTY(EditDefaultsOnly, Category = "VA|Input")
	TObjectPtr<UVAInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, Category = "VA|Input")
	TObjectPtr<UInputAction> MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "VA|Input")
	TObjectPtr<UInputAction> LookInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "VA|Input")
	TObjectPtr<UInputAction> LockOnInputAction;
	
	UFUNCTION(BlueprintCallable, Category = "VA|Combat")
	void ApplyHitStop(float Duration = 0.06f, float TimeDilation = 0.01f);
	
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override { TeamId = NewTeamId; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UVAAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UVAAttributeSet> AttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void InitializeAbilitySystem();
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void OnAbilityInputPressed(FGameplayTag InputTag);
	void OnAbilityInputReleased(FGameplayTag InputTag);
	
	virtual UInputComponent* CreatePlayerInputComponent() override;
	
	// Combat Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VA|Combat")
	TObjectPtr<UVACombatComponent> CombatComponent;
	
	// Target Lock Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VA|Combat")
	TObjectPtr<UVATargetLockComponent> TargetLockComponent;
	
	void MoveAction(const FInputActionValue& Value);
	void LookAction(const FInputActionValue& Value);
	
	void LockOnAction(const FInputActionValue& Value);
	
	UPROPERTY(EditDefaultsOnly, Category = "VA|Team")
	FGenericTeamId TeamId = FGenericTeamId(0); // 0 = Oyuncu, 1 = Düşman
	
};
