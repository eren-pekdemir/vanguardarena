// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/VAAssetManager.h"
#include "VAGameplayTags.h"

void UVAAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	FVAGameplayTags::InitializeNativeTags();

}
