// VAEnemyCharacterBase.cpp
#include "Characters/VAEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AI/VAAIController.h"
#include "VAGameplayTags.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/VASpawnManager.h"
#include "WaveSystem/VAWaveDirector.h"

AVAEnemyCharacter::AVAEnemyCharacter()
{
	// AI Controller sınıfını ayarla
	AIControllerClass = AVAAIController::StaticClass();

	// AI tarafından otomatik possess edilsin
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	TeamId = FGenericTeamId(1); 
	AIControllerClass = AVAAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	HealthBarComponent = CreateDefaultSubobject<UVAHealthBarComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent());
	HealthBarComponent->SetRelativeLocation(FVector(0, 0, 120));
}

void AVAEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// GAS ability'leri ve stat'ları uygula
	GiveStartupAbilities();
	ApplyDefaultStats();
}

void AVAEnemyCharacter::GiveStartupAbilities()
{
	if (!AbilitySystemComponent) return;

	for (TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}

}

void AVAEnemyCharacter::ApplyDefaultStats()
{
	if (!AbilitySystemComponent || !DefaultStats) return;

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
		DefaultStats, 1, Context);

	if (Spec.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void AVAEnemyCharacter::HandleDeath()
{
	if (AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Dead))
	{
		return;
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(FVAGameplayTags::Get().State_Dead);
		AbilitySystemComponent->CancelAllAbilities();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC && AIC->GetBrainComponent())
	{
		AIC->GetBrainComponent()->StopLogic(TEXT("Dead"));
	}

	TArray<AActor*> SpawnManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVASpawnManager::StaticClass(), SpawnManagers);
	for (AActor* SM : SpawnManagers)
	{
		AVASpawnManager* SpawnMgr = Cast<AVASpawnManager>(SM);
		if (SpawnMgr)
		{
			SpawnMgr->OnEnemyDied(this);
		}
	}
	
	TArray<AActor*> WaveDirectors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVAWaveDirector::StaticClass(), WaveDirectors);
	for (AActor* WD : WaveDirectors)
	{
		AVAWaveDirector* Director = Cast<AVAWaveDirector>(WD);
		if (Director)
		{
			Director->OnEnemyDied(this);
		}
	}
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}

	SetLifeSpan(5.0f);
}