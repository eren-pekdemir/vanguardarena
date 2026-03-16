// VAEnemyCharacterBase.h
// Tüm AI düşman karakterlerinin base sınıfı
// VACharacterBase'den türer → GAS otomatik gelir

#pragma once

#include "CoreMinimal.h"
#include "Characters/VACharacterBase.h"
#include "VAEnemyCharacter.generated.h"

UCLASS()
class VANGUARDARENA_API AVAEnemyCharacter : public AVACharacterBase
{
	GENERATED_BODY()

public:
	AVAEnemyCharacter();
	
	UFUNCTION(BlueprintCallable, Category = "VA|AI")
	void HandleDeath();

protected:
	virtual void BeginPlay() override;
	
	// AI başlangıç stat'ları — blueprint'te atanır
	UPROPERTY(EditDefaultsOnly, Category = "VA|AI|Stats")
	TSubclassOf<UGameplayEffect> DefaultStats;

	// Ability'leri ver
	void GiveStartupAbilities();

	// Stat'ları uygula
	void ApplyDefaultStats();
	

};