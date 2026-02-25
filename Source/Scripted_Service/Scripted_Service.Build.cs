// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Scripted_Service : ModuleRules
{
	public Scripted_Service(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "AIModule", "NavigationSystem" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.AddRange(
                    new string[] {
                        "Scripted_Service/Data",
                        "Scripted_Service/Restaurant",
                        "Scripted_Service/Robot",
                        "Scripted_Service/Cooking",
                        "Scripted_Service/UI",
                        "Scripted_Service/Interface"
                    }
                );
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
