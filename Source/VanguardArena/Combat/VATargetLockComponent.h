// VATargetLockComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VATargetLockComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLockChanged, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VANGUARDARENA_API UVATargetLockComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVATargetLockComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    void ToggleLockOn();

    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    void DisableLockOn();

    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    bool IsLockedOn() const { return bIsLockedOn; }

    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    AActor* GetLockedTarget() const { return LockedTarget; }

    UPROPERTY(BlueprintAssignable, Category = "VA|TargetLock")
    FOnTargetLockChanged OnTargetLockChanged;

    UPROPERTY(EditDefaultsOnly, Category = "VA|TargetLock")
    float SearchRadius = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "VA|TargetLock")
    float MaxLockDistance = 2500.0f;

    // Kamera yaw interp hızı (düşük = oyuncu daha çok kontrol eder)
    UPROPERTY(EditDefaultsOnly, Category = "VA|TargetLock")
    float YawInterpSpeed = 3.0f;

private:
    bool bIsLockedOn = false;

    UPROPERTY()
    TObjectPtr<AActor> LockedTarget = nullptr;

    // Önceki controller rotation (smooth için)
    bool bFirstTickAfterLock = false;

    AActor* FindBestTarget() const;
    bool IsTargetValid(AActor* Target) const;
    ACharacter* GetOwnerCharacter() const;
};