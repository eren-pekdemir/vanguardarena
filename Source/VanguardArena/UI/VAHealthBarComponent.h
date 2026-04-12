// VAHealthBarComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "VAHealthBarComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VANGUARDARENA_API UVAHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UVAHealthBarComponent();

	// Health değiştiğinde çağır
	UFUNCTION(BlueprintCallable, Category = "VA|UI")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);

protected:
	virtual void BeginPlay() override;
};