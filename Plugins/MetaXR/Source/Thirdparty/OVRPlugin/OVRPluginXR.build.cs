// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OVRPluginXR : ModuleRules
{
    public OVRPluginXR(ReadOnlyTargetRules Target) : base(Target)
    {




		Type = ModuleType.External;

        string SourceDirectory = "$(PluginDir)/Source/Thirdparty/OVRPlugin/OVRPlugin/";

        PublicIncludePaths.Add(SourceDirectory + "Include");

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            RuntimeDependencies.Add(SourceDirectory + "Lib/arm64-v8a/OpenXR/libOVRPlugin.so");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            RuntimeDependencies.Add(SourceDirectory + "Lib/Mac/OpenXR/OVRPlugin.dylib");
        }
    }
}
