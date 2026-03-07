#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VACombatComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VANGUARDARENA_API UVACombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVACombatComponent();
    
    UPROPERTY(BlueprintReadOnly, Category = "VA|Combo")
    int32 ComboIndex = 0;
    
    UPROPERTY(BlueprintReadOnly, Category = "VA|Combo")
    bool bComboWindowOpen = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "VA|Combo")
    bool bWantsCombo = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Combo")
    int32 MaxComboCount = 3;
    
    UFUNCTION(BlueprintCallable, Category = "VA|Combo")
    void OpenComboWindow();
    
    UFUNCTION(BlueprintCallable, Category = "VA|Combo")
    void CloseComboWindow();
    
    UFUNCTION(BlueprintCallable, Category = "VA|Combo")
    void RequestCombo();
    
    UFUNCTION(BlueprintCallable, Category = "VA|Combo")
    void ResetCombo();
    
    UFUNCTION(BlueprintCallable, Category = "VA|Combo")
    void AdvanceCombo();
    
    UPROPERTY(BlueprintReadOnly, Category = "VA|Combo")
    bool bIsAttacking = false;
};