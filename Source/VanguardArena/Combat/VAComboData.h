#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VAComboData.generated.h"

class UAnimMontage;
class UGameplayEffect;


USTRUCT(BlueprintType)
struct FVAComboAttack
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> AttackMontage = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float DamageMultiplier = 1.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float PlayRate = 1.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> OverrideDamageEffect;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float OverrideTraceRadius = 0.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float OverrideTraceDistance = 0.0f;
};


UCLASS(BlueprintType)
class VANGUARDARENA_API UVAComboDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VA|Combo")
    TArray<FVAComboAttack> ComboAttacks;
    
    const FVAComboAttack* GetComboAttack(int32 Index) const
    {
        if (ComboAttacks.IsValidIndex(Index))
        {
            return &ComboAttacks[Index];
        }
        return nullptr;
    }
    
    int32 GetComboCount() const
    {
        return ComboAttacks.Num();
    }
};