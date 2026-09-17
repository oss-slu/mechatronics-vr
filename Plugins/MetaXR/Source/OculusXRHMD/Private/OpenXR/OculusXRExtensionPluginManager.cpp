// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRExtensionPluginManager.h"

namespace OculusXR
{
	FExtensionPluginManager::FExtensionPluginManager()
		: CoreExtensionPlugin()
		, PerformanceExtensionPlugin()
		, XRSimulatorExtensionPlugin()
		, GuardianExtensionPlugin()
		, LayerExtensionPlugin()
		, DebugUtilsExtensionPlugin()
#ifdef WITH_OCULUS_BRANCH
		, EnvironmentDepthExtensionPlugin()
#endif
#if (defined(WITH_OCULUS_BRANCH) || defined(WITH_OPENXR_BRANCH)) && UE_VERSION_OLDER_THAN(5, 7, 0)
		, SpaceWarpExtensionPlugin()
#endif // defined(WITH_OCULUS_BRANCH)
		, SystemInfoExtensionPlugin()
	{
	}

	FExtensionPluginManager::~FExtensionPluginManager()
	{
	}

	void FExtensionPluginManager::StartupOpenXRPlugins()
	{
		CoreExtensionPlugin.RegisterOpenXRExtensionPlugin();
		PerformanceExtensionPlugin.RegisterOpenXRExtensionPlugin();
		XRSimulatorExtensionPlugin.RegisterOpenXRExtensionPlugin();
		SystemInfoExtensionPlugin.RegisterOpenXRExtensionPlugin();
		GuardianExtensionPlugin.RegisterOpenXRExtensionPlugin();
		LayerExtensionPlugin.RegisterOpenXRExtensionPlugin();
		DebugUtilsExtensionPlugin.RegisterOpenXRExtensionPlugin();
#ifdef WITH_OCULUS_BRANCH
		EnvironmentDepthExtensionPlugin.RegisterOpenXRExtensionPlugin();
#endif
#if (defined(WITH_OCULUS_BRANCH) || defined(WITH_OPENXR_BRANCH)) && UE_VERSION_OLDER_THAN(5, 7, 0)
		SpaceWarpExtensionPlugin.RegisterOpenXRExtensionPlugin();
#endif // defined(WITH_OCULUS_BRANCH)
		MultiPlayerStateExtensionPlugin.RegisterOpenXRExtensionPlugin();
	}

	FPerformanceExtensionPlugin& FExtensionPluginManager::GetPerformanceExtensionPlugin()
	{
		return PerformanceExtensionPlugin;
	}

	FSystemInfoExtensionPlugin& FExtensionPluginManager::GetSystemInfoExtensionPlugin()
	{
		return SystemInfoExtensionPlugin;
	}

	FGuardianExtensionPlugin& FExtensionPluginManager::GetGuardianExtensionPlugin()
	{
		return GuardianExtensionPlugin;
	}

	FLayerExtensionPlugin& FExtensionPluginManager::GetLayerExtensionPlugin()
	{
		return LayerExtensionPlugin;
	}

	FDebugUtilsExtensionPlugin& FExtensionPluginManager::GetDebugUtilsExtensionPlugin()
	{
		return DebugUtilsExtensionPlugin;
	}

#ifdef WITH_OCULUS_BRANCH
	FEnvironmentDepthExtensionPlugin& FExtensionPluginManager::GetEnvironmentDepthExtensionPlugin()
	{
		return EnvironmentDepthExtensionPlugin;
	}
#endif // WITH_OCULUS_BRANCH

	FMultiPlayerStateExtensionPlugin& FExtensionPluginManager::GetMultiPlayerStateExtensionPlugin()
	{
		return MultiPlayerStateExtensionPlugin;
	}

} // namespace OculusXR
