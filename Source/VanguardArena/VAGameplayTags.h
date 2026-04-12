// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FVAGameplayTags
{
public:
	static const FVAGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeTags();
	
	FGameplayTag InputTag_LightAttack;    
	FGameplayTag InputTag_HeavyAttack;    
	FGameplayTag InputTag_Ability1;       
	FGameplayTag InputTag_Ability2;       
	FGameplayTag InputTag_Ultimate;       
	FGameplayTag InputTag_Dodge;
	FGameplayTag InputTag_LockOn;  
	
	FGameplayTag Ability_Attack_Light;
	FGameplayTag Ability_Attack_Heavy;
	FGameplayTag Ability_Skill1;
	FGameplayTag Ability_Skill2;
	FGameplayTag Ability_Ultimate;
	FGameplayTag Ability_Dodge;
	FGameplayTag Ability_Attack_Light2;
	
	FGameplayTag State_Dead;
	FGameplayTag State_Stunned;
	FGameplayTag State_Knockback;
	FGameplayTag State_Invincible;       
	FGameplayTag State_Channeling;       
	FGameplayTag State_Attacking;
	FGameplayTag State_Dodging;
	FGameplayTag State_HeavyAttacking;
	
	FGameplayTag Damage_Physical;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Ice;
	FGameplayTag Damage_Lightning;
	
	FGameplayTag Buff_AttackPower;
	FGameplayTag Buff_AttackSpeed;
	FGameplayTag Buff_Armor;
	FGameplayTag Buff_Regen;
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Slow;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Poison;
	
	FGameplayTag GameplayCue_Hit_Physical;
	FGameplayTag GameplayCue_Hit_Fire;
	FGameplayTag GameplayCue_Buff_Armor;
	FGameplayTag GameplayCue_Debuff_Burn;
	FGameplayTag GameplayCue_Death;
	
	FGameplayTag Event_Montage_MeleeHit; 
	FGameplayTag Event_Montage_ComboWindow;
	FGameplayTag Event_HitReact;       
	
	FGameplayTag Cooldown_Ability_Attack_Light;
	FGameplayTag Cooldown_Ability_Attack_Heavy;
	FGameplayTag Cooldown_Ability_Skill1;
	FGameplayTag Cooldown_Ability_Skill2;
	FGameplayTag Cooldown_Ability_Ultimate;
	FGameplayTag Cooldown_Ability_Dodge;
	FGameplayTag Cooldown_Ability_Elite_Smash;
	
	FGameplayTag Ability_Boss_ComboAttack;
	FGameplayTag Ability_Boss_AOESlam;
	FGameplayTag Ability_Elite_Smash;
	
	FGameplayTag Event_Montage_SpawnProjectile;
	FGameplayTag Event_Montage_AOE;
	FGameplayTag Event_Montage_Smash;
	
private:
	static FVAGameplayTags GameplayTags;
	
	void AddTag(FGameplayTag& OutTag, const FString& TagName, const FString& TagComment);
	
	void AddAllTags();
	
};

