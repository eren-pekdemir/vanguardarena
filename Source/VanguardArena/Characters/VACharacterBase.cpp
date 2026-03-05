// Fill out your copyright notice in the Description page of Project Settings.


#include "VACharacterBase.h"
#include "Input/VAEnhancedInputComponent.h"
#include "Input/VAInputConfig.h"
#include "VAGameplayTags.h"
#include "InputActionValue.h"


// Sets default values
AVACharacterBase::AVACharacterBase()
{
	AbilitySystemComponent = CreateDefaultSubobject<UVAAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UVAAttributeSet>(TEXT("AttributeSet"));
}

void AVACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Hangisi null, bul:
	UVAEnhancedInputComponent* VAInput = Cast<UVAEnhancedInputComponent>(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC)
	{
		if (MoveInputAction)
		{
			EIC->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AVACharacterBase::MoveAction);
		}
		if (LookInputAction)
		{
			EIC->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AVACharacterBase::LookAction);
		}
	}

	if (!InputConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("VA: InputConfig NULL!"));
		return;
	}

	VAInput->BindAbilityActions(
		InputConfig, this,
		&AVACharacterBase::OnAbilityInputPressed,
		&AVACharacterBase::OnAbilityInputReleased
	);

	UE_LOG(LogTemp, Log, TEXT("VA: Ability input'lari baglandi."));
}

void AVACharacterBase::MoveAction(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AVACharacterBase::LookAction(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
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

UInputComponent* AVACharacterBase::CreatePlayerInputComponent()
{
	return NewObject<UVAEnhancedInputComponent>(this, TEXT("InputComponent0"));
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
