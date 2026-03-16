// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VanguardArena : ModuleRules
{
	public VanguardArena(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"VanguardArena",
			"VanguardArena/AI",
			"VanguardArena/Characters",
			"VanguardArena/Variant_Platforming",
			"VanguardArena/Variant_Platforming/Animation",
			"VanguardArena/Variant_Combat",
			"VanguardArena/Variant_Combat/AI",
			"VanguardArena/Variant_Combat/Animation",
			"VanguardArena/Variant_Combat/Gameplay",
			"VanguardArena/Variant_Combat/Interfaces",
			"VanguardArena/Variant_Combat/UI",
			"VanguardArena/Variant_SideScrolling",
			"VanguardArena/Variant_SideScrolling/AI",
			"VanguardArena/Variant_SideScrolling/Gameplay",
			"VanguardArena/Variant_SideScrolling/Interfaces",
			"VanguardArena/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
