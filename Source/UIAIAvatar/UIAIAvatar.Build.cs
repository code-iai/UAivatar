// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class UIAIAvatar : ModuleRules
{
	public UIAIAvatar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule",
            "UPIDController","UROSBridge","Json","JsonUtilities", "UTags", "UConversions", "UIds", "ProceduralMeshComponent",
            "UIAISimUtils",
            "RawMesh", "UDTWPlugin"});


        PrivateDependencyModuleNames.AddRange(new string[] {
			"HeadMountedDisplay",
            "NavigationSystem",
            "UIAISimUtils",
			"SteamVR"
			});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
