// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_AOS : ModuleRules
{
	public Project_AOS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
            "NavigationSystem",
            "AIModule", "" +
            "GameplayTasks",
            "UMG",
            "GameplayTags",
            "OnlineSubsystem",
            "OnlineSubsystemEOS",
            "OnlineSubsystemUtils",
            "Slate", 
			"SlateCore"
        });
	}
}
