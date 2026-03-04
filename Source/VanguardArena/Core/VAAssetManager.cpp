// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/VAAssetManager.h"
#include "VAGameplayTags.h"

void UVAAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	FVAGameplayTags::InitializeNativeTags();

	UE_LOG(LogTemp, Log, TEXT("VAAssetManager: Native GameplayTags kaydedildi."));
}
