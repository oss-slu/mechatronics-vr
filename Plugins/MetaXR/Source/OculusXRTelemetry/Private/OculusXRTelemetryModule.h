// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "OculusXRTelemetryWrapper.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "OculusXRTelemetry"

DECLARE_LOG_CATEGORY_EXTERN(LogOculusXRTelemetry, Log, All);

// Use the HMD module's supported platforms definition if available, otherwise define our own
#ifndef OCULUS_HMD_SUPPORTED_PLATFORMS
#define OCULUS_HMD_SUPPORTED_PLATFORMS ((PLATFORM_WINDOWS && WINVER > 0x0502) || (PLATFORM_ANDROID_ARM || PLATFORM_ANDROID_ARM64 || PLATFORM_ANDROID_X64) || PLATFORM_MAC || PLATFORM_LINUX)
#endif

//-------------------------------------------------------------------------------------------------
// FOculusXRMovementModule
//-------------------------------------------------------------------------------------------------

class FOculusXRTelemetryModule : public IModuleInterface
{
public:
	FOculusXRTelemetryModule();

	static inline FOculusXRTelemetryModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FOculusXRTelemetryModule>("OculusXRTelemetry");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("OculusXRTelemetry");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if OCULUS_HMD_SUPPORTED_PLATFORMS
	static OCULUSXRTELEMETRY_API OculusXR::EngineTelemetryPlugin& GetTelemetryPlugin();
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS

private:
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	static OculusXR::EngineTelemetryPlugin TelemetryPlugin;
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
};

#undef LOCTEXT_NAMESPACE
