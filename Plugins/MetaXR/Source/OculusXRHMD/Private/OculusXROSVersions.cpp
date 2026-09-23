// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXROSVersions.h"

#if WITH_EDITOR
#include <Interfaces/IPluginManager.h>
#endif

constexpr int32 OSVersionOffset = 32;

FOculusXROSVersion::FOculusXROSVersion()
	: Version(0)
	, bLatest(false)
{
}

int32 FOculusXROSVersion::GetVersion() const
{
	return bLatest ? FOculusXROSVersionHelpers::GetOSVersion() : Version;
}

FString FOculusXROSVersionHelpers::GetVersionStringFromPlugin()
{
#if WITH_EDITOR
	FString PluginName = "OculusXR";

	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("OculusXR"));
	if (Plugin.IsValid())
	{
		const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
		return Descriptor.VersionName;
	}
#endif // WITH_EDITOR

	return "";
}

int32 FOculusXROSVersionHelpers::GetOSVersion()
{
#if WITH_EDITOR
	auto VerStr = GetVersionStringFromPlugin();
	int32 FirstPeriod = INDEX_NONE;
	int32 LastPeriod = INDEX_NONE;
	VerStr.FindChar('.', FirstPeriod);
	VerStr.FindLastChar('.', LastPeriod);
	if (FirstPeriod == INDEX_NONE || LastPeriod == INDEX_NONE)
	{
		return 0;
	}

	VerStr.MidInline(FirstPeriod + 1, LastPeriod - FirstPeriod - 1, EAllowShrinking::No);
	const int32 PluginVersion = FCString::Atoi(*VerStr);
	// Starting from v200, OS and plugin versions are the same
	return PluginVersion >= 200 ? PluginVersion : PluginVersion - OSVersionOffset;
#else
	return 0;
#endif // WITH_EDITOR
}
