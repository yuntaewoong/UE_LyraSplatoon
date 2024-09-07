// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LyraSplatoonRuntime : ModuleRules
{
	public LyraSplatoonRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
                
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "LyraGame",
                "GameplayAbilities", 
				"GameplayTags",		 
				"GameplayTasks",
                "AIModule",
				"ComputeShader",
                 "RHI",                   
				"RenderCore",            
				"Renderer", 
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "GameplayAbilities",
                "CommonUI",
                "UMG",
                "DataRegistry",
                "AsyncMixin",
                "EnhancedInput",
                "GameSubtitles",
                "DeveloperSettings",
                "AIModule",
				// ... add private dependencies that you statically link with here ...	
			}
		);
    }
}
