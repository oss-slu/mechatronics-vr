// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once
#include "CoreMinimal.h"
#include "OculusXREnvironmentDepthExtensionPlugin.h"
#include "OculusXRCoreExtensionPlugin.h"
#include "OculusXRDebugUtilsExtensionPlugin.h"
#include "OculusXRGuardianExtensionPlugin.h"
#include "OculusXRLayerExtensionPlugin.h"
#include "OculusXRPerformanceExtensionPlugin.h"
#include "OculusXRSimulatorExtensionPlugin.h"
#include "OculusXRSpaceWarp.h"
#include "OculusXRSystemInfoExtensionPlugin.h"
#include "OculusXRMultiPlayerStateExtensionPlugin.h"

namespace OculusXR
{

	class FExtensionPluginManager
	{
	public:
		FExtensionPluginManager();
		virtual ~FExtensionPluginManager();

		void StartupOpenXRPlugins();

		FPerformanceExtensionPlugin& GetPerformanceExtensionPlugin();
		FSystemInfoExtensionPlugin& GetSystemInfoExtensionPlugin();
		FGuardianExtensionPlugin& GetGuardianExtensionPlugin();
		FLayerExtensionPlugin& GetLayerExtensionPlugin();
		FDebugUtilsExtensionPlugin& GetDebugUtilsExtensionPlugin();
#ifdef WITH_OCULUS_BRANCH
		FEnvironmentDepthExtensionPlugin& GetEnvironmentDepthExtensionPlugin();
#endif
		FMultiPlayerStateExtensionPlugin& GetMultiPlayerStateExtensionPlugin();

	private:
		FCoreExtensionPlugin CoreExtensionPlugin;
		FPerformanceExtensionPlugin PerformanceExtensionPlugin;
		FXRSimulatorExtensionPlugin XRSimulatorExtensionPlugin;
		FGuardianExtensionPlugin GuardianExtensionPlugin;
		FLayerExtensionPlugin LayerExtensionPlugin;
		FDebugUtilsExtensionPlugin DebugUtilsExtensionPlugin;
#ifdef WITH_OCULUS_BRANCH
		FEnvironmentDepthExtensionPlugin EnvironmentDepthExtensionPlugin;
#endif
#if (defined(WITH_OCULUS_BRANCH) || defined(WITH_OPENXR_BRANCH)) && UE_VERSION_OLDER_THAN(5, 7, 0)
		FSpaceWarpExtensionPlugin SpaceWarpExtensionPlugin;
#endif // defined(WITH_OCULUS_BRANCH)
		FMultiPlayerStateExtensionPlugin MultiPlayerStateExtensionPlugin;

		FSystemInfoExtensionPlugin SystemInfoExtensionPlugin;
	};

} // namespace OculusXR
