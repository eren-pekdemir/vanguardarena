// VABossCharacter.cpp
#include "Characters/VABossCharacter.h"
#include "AbilitySystem/VAAbilitySystemComponent.h"
#include "AbilitySystem/VAAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "VAGameplayTags.h"
#include "AIController.h"
#include "BrainComponent.h"

AVABossCharacter::AVABossCharacter()
{
	// Boss daha büyük
	GetCapsuleComponent()->SetCapsuleHalfHeight(120.0f);
	GetCapsuleComponent()->SetCapsuleRadius(60.0f);

	// Boss daha yavaş ama güçlü
	GetCharacterMovement()->MaxWalkSpeed = 350.0f;
}

void AVABossCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentPhase = EVABossPhase::Phase1_Melee;
}

void AVABossCharacter::SetPhase(EVABossPhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;
	if (CurrentPhase == EVABossPhase::Dead) return;

	EVABossPhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;


	// Phase buff uygula
	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);

		TSubclassOf<UGameplayEffect> BuffToApply = nullptr;

		if (NewPhase == EVABossPhase::Phase2_AOE && Phase2BuffEffect)
		{
			BuffToApply = Phase2BuffEffect;
		}
		else if (NewPhase == EVABossPhase::Phase3_Enraged && Phase3BuffEffect)
		{
			BuffToApply = Phase3BuffEffect;
		}

		if (BuffToApply)
		{
			FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
				BuffToApply, 1, Context);
			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	// Phase geçiş montage
	if (PhaseTransitionMontage)
	{
		UAnimInstance* Anim = GetMesh()->GetAnimInstance();
		if (Anim)
		{
			// Geçiş sırasında kısa invincibility
			if (AbilitySystemComponent)
			{
				AbilitySystemComponent->AddLooseGameplayTag(
					FVAGameplayTags::Get().State_Invincible);
			}

			bIsTransitioning = true;
			float Duration = Anim->Montage_Play(PhaseTransitionMontage, 1.0f);

			// Montage bitince invincibility kaldır
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle,
				[this]()
				{
					bIsTransitioning = false;
					if (AbilitySystemComponent)
					{
						AbilitySystemComponent->RemoveLooseGameplayTag(
							FVAGameplayTags::Get().State_Invincible);
					}
				},
				Duration, false);
		}
	}

	// Phase 2 ve 3'te minion çağır
	if (NewPhase == EVABossPhase::Phase2_AOE || NewPhase == EVABossPhase::Phase3_Enraged)
	{
		SpawnMinions();
	}

	// Delegate tetikle (UI güncellemesi için)
	OnBossPhaseChanged.Broadcast(NewPhase);
}

void AVABossCharacter::SpawnMinions()
{
	if (!MinionClass) return;

	FVector BossLocation = GetActorLocation();

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	for (int32 i = 0; i < MinionSpawnCount; i++)
	{
		// Boss etrafında rastgele konum
		float Angle = (360.0f / MinionSpawnCount) * i;
		float Radians = FMath::DegreesToRadians(Angle);
		FVector Offset(FMath::Cos(Radians) * 400.0f, FMath::Sin(Radians) * 400.0f, 0.0f);
		FVector SpawnLoc = BossLocation + Offset;

		// NavMesh'e project et
		if (NavSys)
		{
			FNavLocation NavLoc;
			if (NavSys->ProjectPointToNavigation(SpawnLoc, NavLoc, FVector(500, 500, 200)))
			{
				SpawnLoc = NavLoc.Location;
			}
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Minion = GetWorld()->SpawnActor<AActor>(MinionClass, FTransform(SpawnLoc), Params);

		if (Minion)
		{
		}
	}
}

void AVABossCharacter::HandleDeath()
{
	CurrentPhase = EVABossPhase::Dead;
	OnBossPhaseChanged.Broadcast(EVABossPhase::Dead);


	// Parent death logic (ragdoll, BT stop, destroy)
	Super::HandleDeath();
}