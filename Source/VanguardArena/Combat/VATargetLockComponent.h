// VATargetLockComponent.h
// Düşman kilitleme sistemi — kamera + hareket kontrolü

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VATargetLockComponent.generated.h"

// Kilit durumu değiştiğinde tetiklenir (UI marker için)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLockChanged, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VANGUARDARENA_API UVATargetLockComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVATargetLockComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ─── ANA FONKSİYONLAR ───

    // Lock-on toggle (TAB tuşu)
    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    void ToggleLockOn();

    // Kilidi kaldır
    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    void DisableLockOn();

    // Hedefler arası geçiş (sağ/sol — mouse wheel veya sağ analog)
    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    void SwitchTarget(float Direction);

    // ─── GETTER'LAR ───

    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    bool IsLockedOn() const { return bIsLockedOn; }

    UFUNCTION(BlueprintCallable, Category = "VA|TargetLock")
    AActor* GetLockedTarget() const { return LockedTarget; }

    // ─── DELEGATE ───
    // UI widget bu delegate'i dinler → marker göster/gizle
    UPROPERTY(BlueprintAssignable, Category = "VA|TargetLock")
    FOnTargetLockChanged OnTargetLockChanged;

    // ─── AYARLAR ───

    // Hedef arama yarıçapı (cm)
    UPROPERTY(EditDefaultsOnly, Category = "VA|TargetLock")
    float SearchRadius = 2000.0f;

    // Maksimum kilit mesafesi — bunun ötesinde kilit düşer
    UPROPERTY(EditDefaultsOnly, Category = "VA|TargetLock")
    float MaxLockDistance = 2500.0f;

    // Kamera dönüş hızı (lock-on sırasında hedefe doğru)
    UPROPERTY(EditDefaultsOnly, Category = "VA|TargetLock")
    float CameraRotationSpeed = 10.0f;

private:
    // Kilitli mi?
    bool bIsLockedOn = false;

    // Kilitli hedef
    UPROPERTY()
    TObjectPtr<AActor> LockedTarget = nullptr;

    // En yakın geçerli hedefi bul
    AActor* FindBestTarget() const;

    // Hedef geçerli mi? (yaşıyor mu, mesafe içinde mi)
    bool IsTargetValid(AActor* Target) const;

    // Kamerayı hedefe doğru döndür
    void UpdateCameraRotation(float DeltaTime);

    // Karakter rotasyonunu hedefe doğru döndür
    void UpdateCharacterRotation(float DeltaTime);

    // Owner character referansı (cache)
    ACharacter* GetOwnerCharacter() const;
};