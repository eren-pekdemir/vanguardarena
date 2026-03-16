// VAAIController.cpp
#include "AI/VAAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AVAAIController::AVAAIController()
{
	// BehaviorTree ve Blackboard component'leri otomatik oluştur
	// AAIController zaten BrainComponent (BehaviorTreeComponent) oluşturabiliyor
	// ama biz explicit yapıyoruz

	// Blackboard
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	// BehaviorTree
	BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

	// ─── AI PERCEPTION ───
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	// Sight config oluştur
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.0f;            // Görüş mesafesi (20 metre)
	SightConfig->LoseSightRadius = 2500.0f;         // Hedefi kaybetme mesafesi
	SightConfig->PeripheralVisionAngleDegrees = 60.0f; // Görüş açısı (120 derece toplam)
	SightConfig->SetMaxAge(5.0f);                   // Hafıza süresi (5sn görmezse unutur)

	// Düşman tespiti — tüm takımları algıla
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	// Perception component'e sight ekle
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

	// Perception güncelleme delegate'ini bağla
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AVAAIController::OnPerceptionUpdated);

	// ─── TEAM ───
	// Team ID 1 = Düşmanlar
	SetGenericTeamId(FGenericTeamId(1));
}

void AVAAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("AIController: BehaviorTree atanmamış!"));
		return;
	}

	// Blackboard'u Behavior Tree'nin data asset'inden initialize et
	if (Blackboard)
	{
		Blackboard->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);

		// Default değerleri ayarla
		Blackboard->SetValueAsFloat(BB_AttackRange, 200.0f);
	}

	// Behavior Tree'yi başlat
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComp)
	{
		BTComp->StartTree(*BehaviorTreeAsset);
		UE_LOG(LogTemp, Log, TEXT("AIController: BT başlatıldı — %s"), *BehaviorTreeAsset->GetName());
	}
}

void AVAAIController::OnUnPossess()
{
	// Behavior Tree durdur
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComp)
	{
		BTComp->StopTree();
	}

	Super::OnUnPossess();
}

void AVAAIController::SetTargetActor(AActor* Target)
{
	if (Blackboard)
	{
		Blackboard->SetValueAsObject(BB_TargetActor, Target);

		if (Target)
		{
			Blackboard->SetValueAsVector(BB_TargetLocation, Target->GetActorLocation());
			UE_LOG(LogTemp, Log, TEXT("AI: Target → %s"), *Target->GetName());
		}
	}
}

AActor* AVAAIController::GetTargetActor() const
{
	if (Blackboard)
	{
		return Cast<AActor>(Blackboard->GetValueAsObject(BB_TargetActor));
	}
	return nullptr;
}

void AVAAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// Oyuncuyu gördü → hedef olarak kaydet
		ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (Actor == PlayerChar)
		{
			SetTargetActor(Actor);
			UE_LOG(LogTemp, Log, TEXT("AI Perception: OYUNCU GÖRÜLDÜ!"));
		}
	}
	else
	{
		// Hedefi kaybetti
		AActor* CurrentTarget = GetTargetActor();
		if (Actor == CurrentTarget)
		{
			// Hedef kaybedildi — son görülen konumu kaydet
			if (Blackboard)
			{
				Blackboard->SetValueAsVector(BB_TargetLocation, Actor->GetActorLocation());
			}
			SetTargetActor(nullptr);
			UE_LOG(LogTemp, Log, TEXT("AI Perception: Hedef KAYBOLDU"));
		}
	}
}

ETeamAttitude::Type AVAAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	// Team kontrolü — IGenericTeamAgentInterface üzerinden
	const IGenericTeamAgentInterface* OtherTeam = Cast<IGenericTeamAgentInterface>(&Other);
	if (OtherTeam)
	{
		FGenericTeamId OtherTeamId = OtherTeam->GetGenericTeamId();
		FGenericTeamId MyTeamId = GetGenericTeamId();

		if (OtherTeamId == MyTeamId)
		{
			return ETeamAttitude::Friendly; // Aynı takım — saldırma
		}
		else
		{
			return ETeamAttitude::Hostile;  // Farklı takım — düşman
		}
	}

	return ETeamAttitude::Neutral;
}