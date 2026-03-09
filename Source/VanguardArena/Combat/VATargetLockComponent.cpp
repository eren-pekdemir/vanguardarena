// VATargetLockComponent.cpp
#include "Combat/VATargetLockComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "VAGameplayTags.h"
#include "Engine/OverlapResult.h"

UVATargetLockComponent::UVATargetLockComponent()
{
    // Lock-on aktifken her frame kamera güncellenmeli
    PrimaryComponentTick.bCanEverTick = true;
    // Başlangıçta tick kapalı — lock-on aktive olunca açılır
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UVATargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsLockedOn || !LockedTarget)
    {
        DisableLockOn();
        return;
    }

    // Hedef hala geçerli mi?
    if (!IsTargetValid(LockedTarget))
    {
        DisableLockOn();
        return;
    }

    // Kamerayı ve karakteri hedefe doğru döndür
    UpdateCameraRotation(DeltaTime);
    UpdateCharacterRotation(DeltaTime);
}

void UVATargetLockComponent::ToggleLockOn()
{
    if (bIsLockedOn)
    {
        DisableLockOn();
    }
    else
    {
        // En yakın hedefi bul
        AActor* BestTarget = FindBestTarget();

        if (BestTarget)
        {
            LockedTarget = BestTarget;
            bIsLockedOn = true;

            // Tick'i aç — kamera güncellemesi başlasın
            SetComponentTickEnabled(true);

            // Karakter hareket modunu değiştir — strafe
            ACharacter* Owner = GetOwnerCharacter();
            if (Owner)
            {
                // OrientRotationToMovement kapat → karakter hareket yönüne değil,
                // controller yönüne bakar (yani hedefe)
                Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
                Owner->bUseControllerRotationYaw = true;
            }

            // Delegate tetikle (UI marker göster)
            OnTargetLockChanged.Broadcast(LockedTarget);

            UE_LOG(LogTemp, Log, TEXT("Lock-On: %s KİLİTLENDİ"), *LockedTarget->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Lock-On: Hedef bulunamadı"));
        }
    }
}

void UVATargetLockComponent::DisableLockOn()
{
    if (!bIsLockedOn) return;

    bIsLockedOn = false;
    LockedTarget = nullptr;

    // Tick'i kapat — gereksiz güncelleme yapma
    SetComponentTickEnabled(false);

    // Karakter hareket modunu geri al — normal
    ACharacter* Owner = GetOwnerCharacter();
    if (Owner)
    {
        Owner->GetCharacterMovement()->bOrientRotationToMovement = true;
        Owner->bUseControllerRotationYaw = false;
    }

    // Delegate tetikle (UI marker gizle)
    OnTargetLockChanged.Broadcast(nullptr);

    UE_LOG(LogTemp, Log, TEXT("Lock-On: KİLİT KALDIRILDI"));
}

void UVATargetLockComponent::SwitchTarget(float Direction)
{
    // Şimdilik basit: kilidi kaldır, yeniden en yakını bul
    // İleride sağ/sol yön bazlı hedef değiştirme eklenebilir
    if (!bIsLockedOn) return;

    AActor* OldTarget = LockedTarget;
    DisableLockOn();
    ToggleLockOn();

    // Aynı hedef tekrar kilitlenirse ve başka hedef varsa alternatif bul
    // Bu basit implementasyon — ileride geliştirilir
}

AActor* UVATargetLockComponent::FindBestTarget() const
{
    ACharacter* Owner = GetOwnerCharacter();
    if (!Owner) return nullptr;

    FVector OwnerLocation = Owner->GetActorLocation();

    // SphereOverlap ile çevredeki pawn'ları bul
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Shape = FCollisionShape::MakeSphere(SearchRadius);

    bool bFound = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        OwnerLocation,
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        Shape
    );

    if (!bFound) return nullptr;

    AActor* BestTarget = nullptr;
    float BestScore = MAX_FLT;

    // Kamera yönü — kameranın baktığı yöne yakın hedefler öncelikli
    FVector CameraForward = FVector::ZeroVector;
    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (PC)
    {
        CameraForward = PC->GetControlRotation().Vector();
    }

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* CandidateActor = Overlap.GetActor();
        if (!CandidateActor || CandidateActor == Owner) continue;

        // GAS entity mi? (ASC var mı)
        UAbilitySystemComponent* TargetASC =
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CandidateActor);
        if (!TargetASC) continue;

        // Ölü mü?
        if (TargetASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Dead))
            continue;

        // Mesafe
        float Distance = FVector::Dist(OwnerLocation, CandidateActor->GetActorLocation());
        if (Distance > SearchRadius) continue;

        // Skor: mesafe + kamera yönüne yakınlık
        // Düşük skor = daha iyi hedef
        FVector DirectionToTarget = (CandidateActor->GetActorLocation() - OwnerLocation).GetSafeNormal();
        float DotProduct = FVector::DotProduct(CameraForward, DirectionToTarget);
        
        // DotProduct: 1.0 = tam önünde, -1.0 = arkasında
        // Mesafeyi ağırlıklı skor: yakın ve önünde olan hedef en iyi
        float Score = Distance * (1.0f - DotProduct * 0.5f);

        if (Score < BestScore)
        {
            BestScore = Score;
            BestTarget = CandidateActor;
        }
    }

    return BestTarget;
}

bool UVATargetLockComponent::IsTargetValid(AActor* Target) const
{
    if (!Target) return false;
    if (Target->IsPendingKillPending()) return false;

    // Mesafe kontrolü
    ACharacter* Owner = GetOwnerCharacter();
    if (!Owner) return false;

    float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
    if (Distance > MaxLockDistance) return false;

    // Ölü mü?
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (TargetASC && TargetASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Dead))
    {
        return false;
    }

    return true;
}

void UVATargetLockComponent::UpdateCameraRotation(float DeltaTime)
{
    ACharacter* Owner = GetOwnerCharacter();
    if (!Owner || !LockedTarget) return;

    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (!PC) return;

    // Hedefe bakış rotasyonu hesapla
    FVector Start = Owner->GetActorLocation();
    FVector End = LockedTarget->GetActorLocation();

    // Hedefin biraz üstüne bak (göğüs/baş hizası)
    End.Z += 50.0f;

    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(Start, End);

    // Mevcut controller rotasyonuyla yumuşak geçiş (lerp)
    FRotator CurrentRotation = PC->GetControlRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, DeltaTime, CameraRotationSpeed);

    // Sadece Yaw ve Pitch güncelle (Roll dokunma)
    PC->SetControlRotation(FRotator(NewRotation.Pitch, NewRotation.Yaw, 0.0f));
}

void UVATargetLockComponent::UpdateCharacterRotation(float DeltaTime)
{
    ACharacter* Owner = GetOwnerCharacter();
    if (!Owner || !LockedTarget) return;

    // Karakter hedefe doğru baksın
    FVector Direction = (LockedTarget->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
    Direction.Z = 0.0f; // Sadece yatay düzlemde

    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRotation = Direction.Rotation();
        FRotator CurrentRotation = Owner->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, CameraRotationSpeed);

        Owner->SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
    }
}

ACharacter* UVATargetLockComponent::GetOwnerCharacter() const
{
    return Cast<ACharacter>(GetOwner());
}