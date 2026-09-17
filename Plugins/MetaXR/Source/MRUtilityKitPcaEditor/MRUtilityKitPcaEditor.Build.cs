// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;

public class MRUtilityKitPcaEditor : ModuleRules
{
    public MRUtilityKitPcaEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        bUseUnity = true;

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            });


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "OculusXRHMD",
                "OculusXRAnchors",
                "OculusXRScene",
                "Json",
                "UnrealEd",
                "RHI",
                "RenderCore",
                "ProceduralMeshComponent",
                "MRUtilityKit",
            });
    }
}
