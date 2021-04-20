// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeistFPS : ModuleRules
{
	public HeistFPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara" });
	}
}
