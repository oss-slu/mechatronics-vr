// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class OculusXRTelemetry : ModuleRules
    {
        public OculusXRTelemetry(ReadOnlyTargetRules Target) : base(Target)
        {
            bUseUnity = false;

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "EngineSettings",
                    "HeadMountedDisplay",
                    "Projects"
                });

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "EngineTelemetry"
                }
            );

            // DLL on windows
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                RuntimeDependencies.Add("$(PluginDir)/Source/Thirdparty/EngineTelemetry/Lib/" + Target.Platform.ToString() + "/EngineTelemetry.dll");
            }
        }
    }
}
