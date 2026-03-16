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

AVAEnemyCharacter::AVAEnemyCharacter()
{
	// AI Controller sınıfını ayarla
	AIControllerClass = AVAAIController::StaticClass();

	// AI tarafından otomatik possess edilsin
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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

	UE_LOG(LogTemp, Log, TEXT("Enemy: %d ability verildi"), StartupAbilities.Num());
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
		UE_LOG(LogTemp, Log, TEXT("Enemy: Default stat'lar uygulandı"));
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

	SetLifeSpan(5.0f);
	UE_LOG(LogTemp, Log, TEXT("Enemy %s: ÖLDÜ"), *GetName());
}