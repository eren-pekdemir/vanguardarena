// Fill out your copyright notice in the Description page of Project Settings.


#include "VACharacterBase.h"


// Sets default values
AVACharacterBase::AVACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UVAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	
	AttributeSet = CreateDefaultSubobject<UVAAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AVACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AVACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void AVACharacterBase::InitializeAbilitySystem()
{
	if (!AbilitySystemComponent) return;
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->GiveStartupAbilities(StartupAbilities);
	AbilitySystemComponent->ApplyStartupEffects(StartupEffects);
}
