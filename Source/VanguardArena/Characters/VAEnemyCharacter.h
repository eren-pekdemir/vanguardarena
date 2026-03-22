// VAEnemyCharacterBase.h
// Tüm AI düşman karakterlerinin base sınıfı
// VACharacterBase'den türer → GAS otomatik gelir

#pragma once

#include "CoreMinimal.h"
#include "Characters/VACharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Abilities/GameplayAbility.h"
#include "VAEnemyCharacter.generated.h"

UCLASS()
class VANGUARDARENA_API AVAEnemyCharacter : public AVACharacterBase
{
	GENERATED_BODY()

public:
	AVAEnemyCharacter();
    
	// Bu düşmanın kullanacağı Behavior Tree (Blueprint'te atanır)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
    
	UFUNCTION(BlueprintCallable, Category = "VA|AI")
	virtual void HandleDeath();

protected:
	virtual void BeginPlay() override;
    
	// AI başlangıç stat'ları
	UPROPERTY(EditDefaultsOnly, Category = "VA|AI|Stats")
	TSubclassOf<UGameplayEffect> DefaultStats;

	// Düşmana verilecek ability'ler (Blueprint'te atanır)
	UPROPERTY(EditDefaultsOnly, Category = "VA|AI")
	TArray<TSubclassOf<UGameplayAbility>> EnemyStartupAbilities;

	void GiveStartupAbilities();
	void ApplyDefaultStats();
};