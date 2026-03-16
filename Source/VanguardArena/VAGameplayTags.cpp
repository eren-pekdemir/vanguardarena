// Fill out your copyright notice in the Description page of Project Settings.


#include "VAGameplayTags.h"
#include "GameplayTagsManager.h"


FVAGameplayTags FVAGameplayTags::GameplayTags;

void FVAGameplayTags::InitializeNativeTags()
{
	GameplayTags.AddAllTags();
}

void FVAGameplayTags::AddTag(FGameplayTag& OutTag, const FString& TagName, const FString& TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(*TagName),  
		TagComment         
	);
}

void FVAGameplayTags::AddAllTags()
{
    // ─── INPUT TAGS ───
    AddTag(InputTag_LightAttack,   "InputTag.LightAttack",   "Sol tık / Gamepad X — Hafif saldırı input'u");
    AddTag(InputTag_HeavyAttack,   "InputTag.HeavyAttack",   "Sağ tık / Gamepad Y — Ağır saldırı input'u");
    AddTag(InputTag_Ability1,      "InputTag.Ability1",      "Q / Gamepad LB — 1. yetenek input'u");
    AddTag(InputTag_Ability2,      "InputTag.Ability2",      "E / Gamepad RB — 2. yetenek input'u");
    AddTag(InputTag_Ultimate,      "InputTag.Ultimate",      "R / Gamepad LB+RB — Ultimate input'u");
    AddTag(InputTag_Dodge,         "InputTag.Dodge",         "Space / Gamepad A — Dodge input'u");
	AddTag(InputTag_LockOn, "InputTag.LockOn", "Tab / R3 — Hedef kilitleme toggle");

    // ─── ABILITY TAGS ───
    AddTag(Ability_Attack_Light,   "Ability.Attack.Light",   "Hafif saldırı yeteneği");
    AddTag(Ability_Attack_Heavy,   "Ability.Attack.Heavy",   "Ağır saldırı yeteneği");
    AddTag(Ability_Skill1,         "Ability.Skill1",         "Karakter 1. aktif yeteneği");
    AddTag(Ability_Skill2,         "Ability.Skill2",         "Karakter 2. aktif yeteneği");
    AddTag(Ability_Ultimate,       "Ability.Ultimate",       "Ultimate yeteneği");
    AddTag(Ability_Dodge,          "Ability.Dodge",          "Dodge / kaçış yeteneği");

    // ─── STATE TAGS ───
    AddTag(State_Dead,             "State.Dead",             "Karakter öldü — ability kullanamaz");
    AddTag(State_Stunned,          "State.Stunned",          "Sersemlemiş — hareket ve ability engelli");
    AddTag(State_Knockback,        "State.Knockback",        "Geri itilme — kontrol kaybı");
    AddTag(State_Invincible,       "State.Invincible",       "Hasar almaz — dodge i-frame");
    AddTag(State_Channeling,       "State.Channeling",       "Kanal açıyor — hareket engelli");
    AddTag(State_Attacking,        "State.Attacking",        "Saldırı animasyonu sırasında");
	AddTag(State_Dodging, "State.Dodging", "Dodge sırasında — WASD engelli");
	
    // ─── DAMAGE TAGS ───
    AddTag(Damage_Physical,        "Damage.Physical",        "Fiziksel hasar türü");
    AddTag(Damage_Fire,            "Damage.Fire",            "Ateş hasarı türü");
    AddTag(Damage_Ice,             "Damage.Ice",             "Buz hasarı türü");
    AddTag(Damage_Lightning,       "Damage.Lightning",       "Yıldırım hasarı türü");

    // ─── BUFF / DEBUFF TAGS ───
    AddTag(Buff_AttackSpeed,       "Buff.AttackSpeed",       "Saldırı hızı artışı aktif");
    AddTag(Buff_Armor,             "Buff.Armor",             "Zırh artışı aktif");
    AddTag(Buff_Regen,             "Buff.Regen",             "Can yenileme aktif");
    AddTag(Debuff_Burn,            "Debuff.Burn",            "Yanma — zamanla hasar");
    AddTag(Debuff_Slow,            "Debuff.Slow",            "Yavaşlama aktif");
    AddTag(Debuff_Stun,            "Debuff.Stun",            "Sersemletme aktif");

    // ─── GAMEPLAY CUE TAGS ───
    AddTag(GameplayCue_Hit_Physical, "GameplayCue.Hit.Physical", "Fiziksel vuruş efekti");
    AddTag(GameplayCue_Hit_Fire,     "GameplayCue.Hit.Fire",     "Ateş vuruşu efekti");
    AddTag(GameplayCue_Buff_Applied, "GameplayCue.Buff.Applied", "Buff uygulama efekti");
    AddTag(GameplayCue_Death,        "GameplayCue.Death",        "Ölüm efekti");

    // ─── EVENT TAGS ───
    AddTag(Event_Montage_MeleeHit,    "Event.Montage.MeleeHit",    "AnimNotify — kılıç vuruş anı");
    AddTag(Event_Montage_ComboWindow, "Event.Montage.ComboWindow", "AnimNotify — combo penceresi açık");
	AddTag(Event_HitReact, "Event.HitReact", "Hasar alındığında hit reaction tetikler");
	
	// ─── COOLDOWN TAGS ───
	AddTag(Cooldown_Ability_Attack_Light, "Cooldown.Ability.Attack.Light", "Light Attack cooldown");
	AddTag(Cooldown_Ability_Attack_Heavy, "Cooldown.Ability.Attack.Heavy", "Heavy Attack cooldown");
	AddTag(Cooldown_Ability_Skill1,      "Cooldown.Ability.Skill1",       "Skill 1 cooldown");
	AddTag(Cooldown_Ability_Skill2,      "Cooldown.Ability.Skill2",       "Skill 2 cooldown");
	AddTag(Cooldown_Ability_Ultimate,    "Cooldown.Ability.Ultimate",     "Ultimate cooldown");
	AddTag(Cooldown_Ability_Dodge,       "Cooldown.Ability.Dodge",        "Dodge cooldown");
}