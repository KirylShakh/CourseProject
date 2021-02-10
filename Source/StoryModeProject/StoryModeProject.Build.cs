// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StoryModeProject : ModuleRules
{
	public StoryModeProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
