// VAHealthBarComponent.cpp
#include "UI/VAHealthBarComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"

UVAHealthBarComponent::UVAHealthBarComponent()
{
	SetWidgetClass(nullptr); // Blueprint'te atanacak
	SetDrawSize(FVector2D(120, 12));
	SetRelativeLocation(FVector(0, 0, 120)); // Karakterin üstünde
	SetWidgetSpace(EWidgetSpace::Screen); // Her zaman kameraya baksın
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetVisibility(true);
}

void UVAHealthBarComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVAHealthBarComponent::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
	UUserWidget* HealthWidget = GetWidget();
	if (!HealthWidget) return;

	UProgressBar* Bar = Cast<UProgressBar>(HealthWidget->GetWidgetFromName(TEXT("HealthBar")));
	if (Bar)
	{
		float Percent = (MaxHealth > 0) ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
		Bar->SetPercent(Percent);
	}
}