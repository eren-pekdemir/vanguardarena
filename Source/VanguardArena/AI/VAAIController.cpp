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

	// Düşman tespiti 
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	
	// ─── DAMAGE CONFIG ───
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageConfig->SetMaxAge(5.0f);

	AIPerceptionComp->ConfigureSense(*DamageConfig);
	
	// ─── HEARING CONFIG ───
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 1500.0f;           // 15m duyma mesafesi
	HearingConfig->SetMaxAge(3.0f);                   // 3sn sonra unut
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

	AIPerceptionComp->ConfigureSense(*HearingConfig);
	// Dominant sense hala Sight — hearing yardımcı

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
	}
	
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn && GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), PlayerPawn);
		GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), PlayerPawn->GetActorLocation());
		float Dist = FVector::Dist(InPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
		GetBlackboardComponent()->SetValueAsFloat(TEXT("DistanceToTarget"), Dist);

		SetFocus(PlayerPawn);
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
	APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn || !Actor || Actor == OwnerPawn) return;

	// Team kontrolü
	const IGenericTeamAgentInterface* OtherTeam = Cast<IGenericTeamAgentInterface>(Actor);
	if (OtherTeam && OtherTeam->GetGenericTeamId() == GetGenericTeamId())
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	// Tek stimulus'a güvenme — tüm stimulus'ları kontrol et
	// HERHANGİ BİRİ hala aktifse → target tut
	FActorPerceptionBlueprintInfo Info;
	AIPerceptionComp->GetActorsPerception(Actor, Info);

	bool bAnySensed = false;
	for (const FAIStimulus& S : Info.LastSensedStimuli)
	{
		if (S.WasSuccessfullySensed())
		{
			bAnySensed = true;
			break;
		}
	}

	if (bAnySensed)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), Actor);
		BB->SetValueAsVector(TEXT("TargetLocation"), Actor->GetActorLocation());
		float Dist = FVector::Dist(OwnerPawn->GetActorLocation(), Actor->GetActorLocation());
		BB->SetValueAsFloat(TEXT("DistanceToTarget"), Dist);
    
		// Hedefe sürekli bak
		SetFocus(Actor);
	}
	else
	{
		AActor* Current = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
		if (Current == Actor)
		{
			BB->SetValueAsVector(TEXT("TargetLocation"), Actor->GetActorLocation());
			BB->ClearValue(TEXT("TargetActor"));
			BB->SetValueAsFloat(TEXT("DistanceToTarget"), 0.0f);
        
			// Focus temizle
			ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}

ETeamAttitude::Type AVAAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	// Other'ın team'ini al
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
	if (!OtherTeamAgent) return ETeamAttitude::Neutral;

	// Aynı takım → Friendly
	if (OtherTeamAgent->GetGenericTeamId() == TeamId)
	{
		return ETeamAttitude::Friendly;
	}

	// Farklı takım → Hostile
	return ETeamAttitude::Hostile;
}

void AVAAIController::SetTargetInBlackboard(AActor* Target)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	if (Target)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), Target);
		BB->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
		float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
		BB->SetValueAsFloat(TEXT("DistanceToTarget"), Dist);
	}
	else
	{
		BB->ClearValue(TEXT("TargetActor"));
		BB->ClearValue(TEXT("TargetLocation"));
		BB->SetValueAsFloat(TEXT("DistanceToTarget"), 0.0f);
	}
}