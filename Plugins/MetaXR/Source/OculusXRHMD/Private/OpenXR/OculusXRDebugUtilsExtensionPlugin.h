// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once
#include "CoreMinimal.h"
#include "OpenXR/IOculusXRExtensionPlugin.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOculusDebugUtilsExtensionPlugin, Log, All);

namespace OculusXR
{
	class FDebugUtilsExtensionPlugin : public IOculusXRExtensionPlugin
	{
	private:
		XrSession Session;
		bool bDebugUtilsExtensionAvailable;
		bool bDebugUtilsRegionProfilingX2Available;
		bool bDebugUtilsRegionProfilingX3Available;

	public:
		FDebugUtilsExtensionPlugin();

		// IOpenXRExtensionPlugin
		virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
		virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
		virtual void PostCreateSession(XrSession InSession) override;

		// Public API
		bool BeginProfilingRegion(const FString& RegionName);
		bool EndProfilingRegion();
	};

} // namespace OculusXR
