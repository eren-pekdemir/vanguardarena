// Fill out your copyright notice in the Description page of Project Settings.


#include "VACharacterBase.h"
#include "Input/VAEnhancedInputComponent.h"
#include "Input/VAInputConfig.h"
#include "VAGameplayTags.h"


// Sets default values
AVACharacterBase::AVACharacterBase(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UVAEnhancedInputComponent>(
		ACharacter::InputComponentName))
{
	AbilitySystemComponent = CreateDefaultSubobject<UVAAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComponent->SetIsReplicated(true);
	
	AttributeSet = CreateDefaultSubobject<UVAAttributeSet>(TEXT("AttributeSet"));
}

void AVACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UVAEnhancedInputComponent* VAInput = Cast<UVAEnhancedInputComponent>(PlayerInputComponent);

	if (VAInput && InputConfig)
	{
	
		VAInput->BindAbilityActions(
			InputConfig,                         
			this,                                
			&AVACharacterBase::OnAbilityInputPressed,   
			&AVACharacterBase::OnAbilityInputReleased   
		);

		UE_LOG(LogTemp, Log, TEXT("VA: Ability input'ları bağlandı."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VA: InputConfig eksik! Character Blueprint'te ayarla."));
	}
}

void AVACharacterBase::OnAbilityInputPressed(FGameplayTag InputTag)
{
	if (!AbilitySystemComponent) return;
	
	for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.Ability)
		{
			UVAGameplayAbility* VAAbility = Cast<UVAGameplayAbility>(Spec.Ability);

			if (VAAbility && VAAbility->InputTag.MatchesTagExact(InputTag))
			{
				AbilitySystemComponent->TryActivateAbility(Spec.Handle);

				UE_LOG(LogTemp, Verbose, TEXT("VA: Ability aktive edildi — Tag: %s"),
					*InputTag.ToString());
				return;
			}
		}
	}
}

void AVACharacterBase::OnAbilityInputReleased(FGameplayTag InputTag)
{
	// Şimdilik boş — ileride hold-release (Heavy Attack charge) için kullanılacak
	// İmza hazır, ihtiyaç olunca implement edilir
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
