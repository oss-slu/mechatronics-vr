using UnrealBuildTool;

public class KhronosOpenXRHeaders : ModuleRules
{
    public KhronosOpenXRHeaders(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string SourceDirectory = "$(PluginDir)/Source/Thirdparty/KhronosOpenXR/";

        PublicIncludePaths.Add(SourceDirectory + "include");
		PublicIncludePaths.Add(SourceDirectory + "src/common");
	}
}

