// VAProjectile.cpp
#include "Combat/VAProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/OverlapResult.h"

AVAProjectile::AVAProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Sphere Collision
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetSphereRadius(20.0f);
	SphereCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SphereCollision->SetGenerateOverlapEvents(true);
	SetRootComponent(SphereCollision);

	// Mesh (görsel — Blueprint'te değiştirilebilir)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(SphereCollision);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Projectile Movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // Yerçekimi yok — düz uçar
}

void AVAProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Hız ayarla (constructor'dan sonra değişmiş olabilir)
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;

	// Overlap callback bağla
	SphereCollision->OnComponentBeginOverlap.AddDynamic(
		this, &AVAProjectile::OnSphereOverlap);

	// Ömür ayarla
	SetLifeSpan(LifeTime);
}

void AVAProjectile::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Çift hasar engelle
	if (bHasHit) return;

	// Kendine çarpma engelle
	if (OtherActor == OwnerActor || OtherActor == GetInstigator()) return;

	bHasHit = true;

	UE_LOG(LogTemp, Log, TEXT("Projectile: %s'ye çarptı"), *OtherActor->GetName());

	if (bExplodeOnImpact)
	{
		// AOE patlama
		ApplyAOEDamage(GetActorLocation());
	}
	else
	{
		// Tek hedef hasar
		ApplyDamageToTarget(OtherActor);
	}

	// Mermiyi yok et
	Destroy();
}

void AVAProjectile::ApplyDamageToTarget(AActor* Target)
{
	if (!Target) return;

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (TargetASC && DamageSpecHandle.IsValid())
	{
		UAbilitySystemComponent* SourceASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);

		if (SourceASC)
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(
				*DamageSpecHandle.Data.Get(), TargetASC);

			UE_LOG(LogTemp, Log, TEXT("Projectile: %s'ye hasar uygulandı"), *Target->GetName());
		}
	}
}

void AVAProjectile::ApplyAOEDamage(const FVector& Location)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Shape = FCollisionShape::MakeSphere(ExplosionRadius);

	GetWorld()->OverlapMultiByChannel(
		Overlaps, Location, FQuat::Identity,
		ECollisionChannel::ECC_Pawn, Shape);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == OwnerActor) continue;

		ApplyDamageToTarget(HitActor);
	}

	UE_LOG(LogTemp, Log, TEXT("Projectile: AOE patlama — yarıçap: %.0f"), ExplosionRadius);
}