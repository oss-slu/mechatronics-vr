// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "khronos/openxr/openxr.h"

#include "CoreMinimal.h"
#include "IOpenXRExtensionPlugin.h"
#include "OpenXRCore.h"
#include "Misc/EngineVersionComparison.h"
#include "khronos/openxr/meta_openxr_preview/meta_simultaneous_hands_and_controllers.h"

namespace OculusXRInput
{

	class FSimultaneousHandsAndControllersExtensionPlugin : public IOpenXRExtensionPlugin
	{

	public:
		void RegisterOpenXRExtensionPlugin()
		{
#if !UE_VERSION_OLDER_THAN(5, 5, 0)
			RegisterOpenXRExtensionModularFeature();
#endif
		}

		bool SetSimultaneousHandsAndControllersEnabled(bool bEnabled);
		bool IsSimultaneousHandsAndControllersEnabled() const;

		// IOpenXRExtensionPlugin
		virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
		virtual void PostCreateSession(XrSession InSession) override;
		virtual const void* OnGetSystem(XrInstance InInstance, const void* InNext) override;
		virtual const void* OnCreateInstance(class IOpenXRHMDModule* InModule, const void* InNext) override;
		virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;

	private:
		XrSession Session = XR_NULL_HANDLE;

		PFN_xrResumeSimultaneousHandsAndControllersTrackingMETA xrResumeSimultaneousHandsAndControllersTrackingMETA = nullptr;
		PFN_xrPauseSimultaneousHandsAndControllersTrackingMETA xrPauseSimultaneousHandsAndControllersTrackingMETA = nullptr;

		bool bIsExtensionSupported = false;
		bool bIsExtensionEnabled = false;
		bool bSimultaneousHandsAndControllersActive = false;
	};
} // namespace OculusXRInput
