// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRSimultaneousHandsAndControllersExtensionPlugin.h"

#include "IOpenXRHMDModule.h"
#include "OpenXRCore.h"

namespace OculusXRInput
{
	bool FSimultaneousHandsAndControllersExtensionPlugin::SetSimultaneousHandsAndControllersEnabled(bool bEnabled)
	{
		bool bSuccess;
		if (bEnabled)
		{
			XrSimultaneousHandsAndControllersTrackingResumeInfoMETA ResumeInfo = {
				XR_TYPE_SIMULTANEOUS_HANDS_AND_CONTROLLERS_TRACKING_RESUME_INFO_META
			};
			const auto Result = xrResumeSimultaneousHandsAndControllersTrackingMETA(Session, &ResumeInfo);
			bSuccess = XR_SUCCEEDED(Result);
			if (bSuccess)
			{
				bSimultaneousHandsAndControllersActive = true;
			}
		}
		else
		{
			XrSimultaneousHandsAndControllersTrackingPauseInfoMETA PauseInfo = {
				XR_TYPE_SIMULTANEOUS_HANDS_AND_CONTROLLERS_TRACKING_PAUSE_INFO_META
			};
			const auto Result = xrPauseSimultaneousHandsAndControllersTrackingMETA(Session, &PauseInfo);
			bSuccess = XR_SUCCEEDED(Result);
			if (bSuccess)
			{
				bSimultaneousHandsAndControllersActive = false;
			}
		}

		return bSuccess;
	}

	bool FSimultaneousHandsAndControllersExtensionPlugin::IsSimultaneousHandsAndControllersEnabled() const
	{
		return bSimultaneousHandsAndControllersActive;
	}

	bool FSimultaneousHandsAndControllersExtensionPlugin::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
	{
		OutExtensions.Add(XR_META_SIMULTANEOUS_HANDS_AND_CONTROLLERS_EXTENSION_NAME);
		return true;
	}

	void FSimultaneousHandsAndControllersExtensionPlugin::PostCreateSession(XrSession InSession)
	{
		IOpenXRExtensionPlugin::PostCreateSession(InSession);
		Session = InSession;

		// Reset state between PIE sessions.  Must be called after session is created or setter will fail.
		bSimultaneousHandsAndControllersActive = false;
		SetSimultaneousHandsAndControllersEnabled(false);
	}

	const void* FSimultaneousHandsAndControllersExtensionPlugin::OnGetSystem(XrInstance InInstance, const void* InNext)
	{
		// Store extension open xr calls to member function pointers for convenient use.
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPauseSimultaneousHandsAndControllersTrackingMETA", (PFN_xrVoidFunction*)&xrPauseSimultaneousHandsAndControllersTrackingMETA));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrResumeSimultaneousHandsAndControllersTrackingMETA", (PFN_xrVoidFunction*)&xrResumeSimultaneousHandsAndControllersTrackingMETA));

		return InNext;
	}

	const void* FSimultaneousHandsAndControllersExtensionPlugin::OnCreateInstance(class IOpenXRHMDModule* InModule, const void* InNext)
	{
		bIsExtensionEnabled = InModule->IsExtensionEnabled(XR_META_SIMULTANEOUS_HANDS_AND_CONTROLLERS_EXTENSION_NAME);
		bSimultaneousHandsAndControllersActive = false;

		return InNext;
	}

	const void* FSimultaneousHandsAndControllersExtensionPlugin::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
	{
		XrSystemSimultaneousHandsAndControllersPropertiesMETA SimultaneousHandsAndControllersProperties{ XR_TYPE_SYSTEM_SIMULTANEOUS_HANDS_AND_CONTROLLERS_PROPERTIES_META };
		XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES, &SimultaneousHandsAndControllersProperties };
		XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &systemProperties));
		bIsExtensionSupported = SimultaneousHandsAndControllersProperties.supportsSimultaneousHandsAndControllers == XR_TRUE;

		return InNext;
	}
} // namespace OculusXRInput
