// Fill out your copyright notice in the Description page of Project Settings.


#include "VAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "VAGameplayTags.h"
#include "Characters/VAEnemyCharacter.h"

UVAAttributeSet::UVAAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitMana(100.0f);
	InitMaxMana(100.0f);
	InitAttackPower(100.0f);
	InitCriticalChance(0.05f);   
	InitCriticalDamage(1.5f);    
	InitArmor(0.0f);             
	InitIncomingDamage(0.0f);    
	InitIncomingHealing(0.0f);
}

void UVAAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, Health, 
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, MaxHealth, 
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, Mana, 
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, MaxMana, 
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, AttackPower, 
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, CriticalChance, 
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, CriticalDamage, 
		COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVAAttributeSet, Armor, 
		COND_None, REPNOTIFY_Always);
}

void UVAAttributeSet::PreAttributeChange(
	const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetCriticalChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	}
	else if (Attribute == GetArmorAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UVAAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();
	
	AActor* SourceActor = nullptr;
	AController* SourceController = nullptr;
	ACharacter* SourceCharacter = nullptr;
	

	
	if (SourceASC && SourceASC->AbilityActorInfo.IsValid() && SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		SourceActor = SourceASC->AbilityActorInfo->AvatarActor.Get();
		SourceController = SourceASC->AbilityActorInfo->PlayerController.Get();
		if (SourceController == nullptr && SourceActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(SourceActor))
			{
				SourceController = Pawn->GetController();
			}
			if (SourceActor)
			{
				SourceCharacter = Cast<ACharacter>(SourceActor);
			}
		}
	}
	
	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	ACharacter* TargetCharacter = nullptr;
	
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		if (APawn* Pawn = Cast<APawn>(TargetActor))
		{
			TargetController = Pawn->GetController();
		}
		TargetCharacter = Cast<ACharacter>(TargetActor);
	}
	
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		float RawDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f);
		if (RawDamage > 0.0f)
		{
			UAbilitySystemComponent* OwnerASC = GetOwningAbilitySystemComponent();
			if (OwnerASC && OwnerASC->HasMatchingGameplayTag(FVAGameplayTags::Get().State_Invincible))
			{
				// Hasarı sıfırla — dokunulmaz
				SetIncomingDamage(0.0f);
				UE_LOG(LogTemp, Log, TEXT("DAMAGE BLOCKED: Target is INVINCIBLE (i-frame)"));
				return; // Hasar pipeline'ını atla
			}
			
			float CurrentArmor = GetArmor();
			float DamageReduction = CurrentArmor / (CurrentArmor + 100.f);
			float FinalDamage = RawDamage * (1.0f - DamageReduction);
			
			FinalDamage = FMath::Max(FinalDamage, 1.0f);
			
			float CurrentHealth = GetHealth();
			float NewHealth = FMath::Max(CurrentHealth - FinalDamage, 0.0f);
			SetHealth(NewHealth);
			
			UE_LOG(LogTemp, Log, 
			   TEXT("[Damage] %s → %s | Raw: %.1f | Armor: %.1f (%.0f%% reduction) | Final: %.1f | HP: %.1f → %.1f"),
			   SourceActor ? *SourceActor->GetName() : TEXT("Unknown"),
			   TargetActor ? *TargetActor->GetName() : TEXT("Unknown"),
			   RawDamage,
			   CurrentArmor,
			   DamageReduction * 100.0f,
			   FinalDamage,
			   CurrentHealth,
			   NewHealth
		   );
			// ─── HIT REACTION TETİKLE ───
			// Hasar aldıktan sonra hedefe hit react event'i gönder
			if (FinalDamage > 0.0f && GetOwningActor())
			{
				// Gameplay Event gönder → GA_HitReact dinliyor
				FGameplayEventData HitReactPayload;
				HitReactPayload.EventTag = FVAGameplayTags::Get().Event_HitReact;

				// Saldıranın bilgisi (yön hesaplaması için)
				if (Data.EffectSpec.GetEffectContext().GetInstigator())
				{
					HitReactPayload.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
				}
				HitReactPayload.Target = GetOwningActor();

				// Event'i target'ın ASC'sine gönder
				UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
				if (TargetASC)
				{
					TargetASC->HandleGameplayEvent(
						FVAGameplayTags::Get().Event_HitReact,
						&HitReactPayload
					);
				}
			}
			
			// Health 0 veya altina dustu -> olum
			if (GetHealth() <= 0.0f)
			{
				SetHealth(0.0f);

				// Ölüm event'i gönder
				// Ölüm işlemini karakter sınıfına bırak (enemy vs player farklı davranır)
				AActor* Owner = GetOwningActor();
				if (Owner)
				{
					// Enemy karakter mi?
					AVAEnemyCharacter* EnemyChar = Cast<AVAEnemyCharacter>(Owner);
					if (EnemyChar)
					{
						EnemyChar->HandleDeath();
					}
					else
					{
						// Oyuncu ölümü — ileride implement edilecek
						// Şimdilik sadece Dead tag ekle
						UAbilitySystemComponent* DeathASC = GetOwningAbilitySystemComponent();
						if (DeathASC)
						{
							DeathASC->AddLooseGameplayTag(FVAGameplayTags::Get().State_Dead);
						}
					}
				}
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
	{
		float HealAmount = GetIncomingHealing();
		SetIncomingHealing(0.0f);
        
		if (HealAmount > 0.0f)
		{
			float NewHealth = FMath::Min(GetHealth() + HealAmount, GetMaxHealth());
			SetHealth(NewHealth);
            
			UE_LOG(LogTemp, Log, 
				TEXT("[Heal] %s healed for %.1f | HP: %.1f"),
				TargetActor ? *TargetActor->GetName() : TEXT("Unknown"),
				HealAmount, NewHealth);
		}
	}
}

void UVAAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, Health, OldHealth);
}

void UVAAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, MaxHealth, OldMaxHealth);
}

void UVAAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, Mana, OldMana);
}

void UVAAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, MaxMana, OldMaxMana);
}

void UVAAttributeSet::OnRep_AttackPower(
	const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, AttackPower, OldAttackPower);
}

void UVAAttributeSet::OnRep_CriticalChance(
	const FGameplayAttributeData& OldCriticalChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, CriticalChance, OldCriticalChance);
}

void UVAAttributeSet::OnRep_CriticalDamage(
	const FGameplayAttributeData& OldCriticalDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, CriticalDamage, OldCriticalDamage);
}

void UVAAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVAAttributeSet, Armor, OldArmor);
}
