// VATargetLockComponent.cpp
#include "Combat/VATargetLockComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "VAGameplayTags.h"

UVATargetLockComponent::UVATargetLockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

	if (!IsTargetValid(LockedTarget))
	{
		DisableLockOn();
		return;
	}

	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner) return;

	APlayerController* PC = Cast<APlayerController>(Owner->GetController());
	if (!PC) return;

	// ─── HEDEFE BAKIŞ AÇISI HESAPLA (SADECE YAW) ───
	FVector OwnerLoc = Owner->GetActorLocation();
	FVector TargetLoc = LockedTarget->GetActorLocation();

	FVector ToTarget = TargetLoc - OwnerLoc;
	ToTarget.Z = 0.0f; // Sadece yatay düzlem

	if (ToTarget.IsNearlyZero()) return;

	float DesiredYaw = ToTarget.Rotation().Yaw;

	// ─── MEVCUT CONTROLLER ROTATION ───
	FRotator CurrentRot = PC->GetControlRotation();

	// ─── İLK FRAME: Ani atlama yapma, sadece flag sıfırla ───
	if (bFirstTickAfterLock)
	{
		bFirstTickAfterLock = false;
		// İlk frame'de büyük açı farkı olabilir, interp'e bırak
	}

	// ─── YAW INTERP — FMath::RInterpTo açı sarmasını düzgün yapar ───
	// FMath::FInterpTo KULLANMA — 350°→10° geçişinde 360 döner!
	// FMath::RInterpTo rotator interp yapar, en kısa yolu seçer
	FRotator DesiredRot(CurrentRot.Pitch, DesiredYaw, 0.0f);
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, YawInterpSpeed);

	// Pitch'i OYUNCUNUN KONTROLÜNDE BIRAK — sadece yaw'ı güncelle
	PC->SetControlRotation(FRotator(CurrentRot.Pitch, NewRot.Yaw, 0.0f));
}

void UVATargetLockComponent::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		DisableLockOn();
		return;
	}

	AActor* Target = FindBestTarget();
	if (!Target)
	{
		UE_LOG(LogTemp, Log, TEXT("LockOn: Hedef bulunamadı"));
		return;
	}

	LockedTarget = Target;
	bIsLockedOn = true;
	bFirstTickAfterLock = true;
	SetComponentTickEnabled(true);

	// Karakter hedefe doğru baksın (hareket yönü değil, controller yönü)
	ACharacter* Owner = GetOwnerCharacter();
	if (Owner)
	{
		Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
		Owner->bUseControllerRotationYaw = true;

		// Karakter rotation hızını ayarla — yumuşak dönüş
		Owner->GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}

	OnTargetLockChanged.Broadcast(LockedTarget);
	UE_LOG(LogTemp, Log, TEXT("LockOn: %s KİLİTLENDİ"), *LockedTarget->GetName());
}

void UVATargetLockComponent::DisableLockOn()
{
	if (!bIsLockedOn) return;

	bIsLockedOn = false;
	LockedTarget = nullptr;
	SetComponentTickEnabled(false);

	ACharacter* Owner = GetOwnerCharacter();
	if (Owner)
	{
		Owner->GetCharacterMovement()->bOrientRotationToMovement = true;
		Owner->bUseControllerRotationYaw = false;

		// Rotation rate geri al
		Owner->GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}

	OnTargetLockChanged.Broadcast(nullptr);
	UE_LOG(LogTemp, Log, TEXT("LockOn: KİLİT KALDIRILDI"));
}

AActor* UVATargetLockComponent::FindBestTarget() const
{
	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner) return nullptr;

	FVector Origin = Owner->GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Shape = FCollisionShape::MakeSphere(SearchRadius);

	bool bFound = GetWorld()->OverlapMultiByChannel(
		Overlaps, Origin, FQuat::Identity, ECollisionChannel::ECC_Pawn, Shape);

	if (!bFound) return nullptr;

	// Kamera ileri yönü
	FVector CamFwd = FVector::ForwardVector;
	APlayerController* PC = Cast<APlayerController>(Owner->GetController());
	if (PC)
	{
		FRotator CamRot = PC->GetControlRotation();
		CamFwd = FRotationMatrix(FRotator(0, CamRot.Yaw, 0)).GetUnitAxis(EAxis::X);
	}

	AActor* Best = nullptr;
	float BestScore = MAX_FLT;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == Owner) continue;

		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (!TargetASC) continue;

		if (TargetASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Dead))
			continue;

		float Dist = FVector::Dist(Origin, Candidate->GetActorLocation());
		if (Dist > SearchRadius || Dist < 1.0f) continue;

		FVector Dir = (Candidate->GetActorLocation() - Origin).GetSafeNormal();
		Dir.Z = 0.0f;
		float Dot = FVector::DotProduct(CamFwd, Dir.GetSafeNormal());

		float Score = Dist * (1.0f - Dot * 0.5f);

		if (Score < BestScore)
		{
			BestScore = Score;
			Best = Candidate;
		}
	}

	return Best;
}

bool UVATargetLockComponent::IsTargetValid(AActor* Target) const
{
	if (!Target || Target->IsPendingKillPending()) return false;

	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner) return false;

	float Dist = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
	if (Dist > MaxLockDistance) return false;

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (TargetASC && TargetASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Dead))
		return false;

	return true;
}

ACharacter* UVATargetLockComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}