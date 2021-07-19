// Some copyright should be here...

using UnrealBuildTool;

public class ExKismetEditor : ModuleRules
{
	public ExKismetEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"ExKismet"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"BlueprintGraph",
				"Engine",
				"InputCore",
                "Kismet",
                "KismetCompiler",
                "KismetWidgets",
                "MovieScene",
				"MovieSceneTools",
				"Sequencer",
				"EditorStyle",
				"UnrealEd",
				"TimeManagement",
				"Projects",
                "WorkspaceMenuStructure",
                "DetailCustomizations",
                "PropertyEditor",
                "GraphEditor",
                "BlueprintGraph",
				// ... add private dependencies that you statically link with here ...	
			}
			);

        PrivateIncludePathModuleNames.AddRange(
        new string[] {
                    "AssetRegistry",
                    "AssetTools",
                    "BlueprintGraph",
                    "Kismet",
                    "KismetCompiler",
                    "KismetWidgets",
                    "EditorWidgets",
                    "ContentBrowser",
					"GraphEditor",
					"BlueprintGraph",
					"KismetCompiler",
					"KismetWidgets"
        });

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
