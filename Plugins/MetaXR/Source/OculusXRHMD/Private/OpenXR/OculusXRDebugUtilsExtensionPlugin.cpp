// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRDebugUtilsExtensionPlugin.h"
#include "OculusXRXRFunctions.h"
#include "OculusXROpenXRUtilities.h"
#include "OpenXRCore.h"
#include "IOpenXRHMD.h"
#include "IOpenXRHMDModule.h"

#include <khronos/openxr/meta_openxr_preview/metax2_debug_utils_region_profiling.h>
#include <khronos/openxr/meta_openxr_preview/metax3_debug_utils_region_profiling.h>

DEFINE_LOG_CATEGORY(LogOculusDebugUtilsExtensionPlugin);

namespace OculusXR
{

	FDebugUtilsExtensionPlugin::FDebugUtilsExtensionPlugin()
		: Session(XR_NULL_HANDLE)
		, bDebugUtilsExtensionAvailable(false)
		, bDebugUtilsRegionProfilingX2Available(false)
		, bDebugUtilsRegionProfilingX3Available(false)
	{
	}

	bool FDebugUtilsExtensionPlugin::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
	{
		return true;
	}

	bool FDebugUtilsExtensionPlugin::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
	{
		// Request the extensions needed for profiling regions
		OutExtensions.Add(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
		OutExtensions.Add(XR_METAX2_DEBUG_UTILS_REGION_PROFILING_EXTENSION_NAME);
		OutExtensions.Add(XR_METAX3_DEBUG_UTILS_REGION_PROFILING_EXTENSION_NAME);
		return true;
	}

	void FDebugUtilsExtensionPlugin::PostCreateSession(XrSession InSession)
	{
		Session = InSession;
	}

	bool FDebugUtilsExtensionPlugin::BeginProfilingRegion(const FString& RegionName)
	{
		// Check if at least one profiling extension is supported
		if (!bDebugUtilsRegionProfilingX2Available && !bDebugUtilsRegionProfilingX3Available)
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Warning, TEXT("BeginProfilingRegion: Neither XR_METAX2_debug_utils_region_profiling nor XR_METAX3_debug_utils_region_profiling is supported"));
			return false;
		}

		if (!bDebugUtilsExtensionAvailable)
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Warning, TEXT("BeginProfilingRegion: XR_EXT_debug_utils is not supported"));
			return false;
		}

		if (Session == XR_NULL_HANDLE)
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Error, TEXT("BeginProfilingRegion: XrSession is not valid"));
			return false;
		}

		if (RegionName.IsEmpty())
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Error, TEXT("BeginProfilingRegion: RegionName cannot be empty"));
			return false;
		}

		// Store the string conversion to persist the data
		const auto RegionNameAnsi = StringCast<ANSICHAR>(*RegionName);

		// Prefer X3 over X2 if available
		if (bDebugUtilsRegionProfilingX3Available)
		{
			// Create the X3 profiling info structure that extends XrDebugUtilsLabelEXT
			XrDebugUtilsRegionProfilingInfoMETAX3 ProfilingInfo = {};
			ProfilingInfo.type = XR_TYPE_DEBUG_UTILS_REGION_PROFILING_INFO_METAX3;
			ProfilingInfo.next = nullptr;
			ProfilingInfo.workloadType = XR_REGION_WORKLOAD_TYPE_SCENE_EXECUTING_METAX3;

			// Create the label structure
			XrDebugUtilsLabelEXT Label = {};
			Label.type = XR_TYPE_DEBUG_UTILS_LABEL_EXT;
			Label.next = &ProfilingInfo; // Chain the profiling info
			Label.labelName = RegionNameAnsi.Get();

			// Call the OpenXR function
			XrResult Result = xrSessionBeginDebugUtilsLabelRegionEXT.GetValue()(Session, &Label);

			if (XR_FAILED(Result))
			{
				UE_LOG(LogOculusDebugUtilsExtensionPlugin, Warning, TEXT("BeginProfilingRegion failed for region '%s' using X3: XrResult=%d"), *RegionName, (int32)Result);
				return false;
			}

			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Verbose, TEXT("BeginProfilingRegion: %s (using X3)"), *RegionName);
			return true;
		}
		else
		{
			// Fallback to X2
			XrDebugUtilsRegionProfilingInfoMETAX2 ProfilingInfo = {};
			ProfilingInfo.type = XR_TYPE_DEBUG_UTILS_REGION_PROFILING_INFO_METAX2;
			ProfilingInfo.next = nullptr;
			ProfilingInfo.workloadType = XR_REGION_WORKLOAD_TYPE_SCENE_EXECUTING_METAX2;

			// Create the label structure
			XrDebugUtilsLabelEXT Label = {};
			Label.type = XR_TYPE_DEBUG_UTILS_LABEL_EXT;
			Label.next = &ProfilingInfo; // Chain the profiling info
			Label.labelName = RegionNameAnsi.Get();

			// Call the OpenXR function
			XrResult Result = xrSessionBeginDebugUtilsLabelRegionEXT.GetValue()(Session, &Label);

			if (XR_FAILED(Result))
			{
				UE_LOG(LogOculusDebugUtilsExtensionPlugin, Warning, TEXT("BeginProfilingRegion failed for region '%s' using X2: XrResult=%d"), *RegionName, (int32)Result);
				return false;
			}

			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Verbose, TEXT("BeginProfilingRegion: %s (using X2)"), *RegionName);
			return true;
		}
	}

	bool FDebugUtilsExtensionPlugin::EndProfilingRegion()
	{
		// Check if at least one profiling extension is supported
		if (!bDebugUtilsRegionProfilingX2Available && !bDebugUtilsRegionProfilingX3Available)
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Warning, TEXT("EndProfilingRegion: Neither XR_METAX2_debug_utils_region_profiling nor XR_METAX3_debug_utils_region_profiling is supported"));
			return false;
		}

		if (!bDebugUtilsExtensionAvailable)
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Warning, TEXT("EndProfilingRegion: XR_EXT_debug_utils is not supported"));
			return false;
		}

		if (Session == XR_NULL_HANDLE)
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Error, TEXT("EndProfilingRegion: XrSession is not valid"));
			return false;
		}

		// Call the OpenXR function (same for both X2 and X3)
		XrResult Result = xrSessionEndDebugUtilsLabelRegionEXT.GetValue()(Session);

		if (XR_FAILED(Result))
		{
			UE_LOG(LogOculusDebugUtilsExtensionPlugin, Warning, TEXT("EndProfilingRegion failed: XrResult=%d"), (int32)Result);
			return false;
		}

		UE_LOG(LogOculusDebugUtilsExtensionPlugin, Verbose, TEXT("EndProfilingRegion"));
		return true;
	}

} // namespace OculusXR
